#include <cstdint>
#include <cstdio>

#include "sd_blocks.h"

#include "log/log.h"
#include "hal/MMCSD/MMCSD.hpp"
#include "hal/MMCSD/Card.hpp"

#define TAG "sd_blk"

namespace
{
    using namespace HAL::MMCSD;

    /*  Scratch area used by the write test.
     *
     *  It sits 4 MiB into the card, well clear of the partition table and of the boot area a
     *  BeagleBone Black image keeps at the very front of the media. The test is still
     *  non-destructive: whatever lives there is read out first and written back at the end.
     */
    constexpr uint32_t SCRATCH_BLOCK   = 8192u;     // 8192 * 512 B = 4 MiB offset
    constexpr uint32_t MULTI_BLOCKS    = 8u;        // number of blocks of the multi block round

    /*  One block of the card, plus room for the multi block round. Kept in .bss rather than on the
     *  stack: the baremetal runtime gives the application a modest stack.
     */
    alignas(4) uint8_t g_original[BLOCK_SIZE * MULTI_BLOCKS];
    alignas(4) uint8_t g_pattern [BLOCK_SIZE * MULTI_BLOCKS];
    alignas(4) uint8_t g_readback[BLOCK_SIZE * MULTI_BLOCKS];

    mmcsd0_t g_mmc0;

    const char* status_name(const Status st) noexcept
    {
        switch (st)
        {
            case Status::OK:              return "OK";
            case Status::NOT_INITIALIZED: return "NOT_INITIALIZED";
            case Status::BAD_ARGUMENT:    return "BAD_ARGUMENT";
            case Status::TIMEOUT:         return "TIMEOUT";
            case Status::CMD_TIMEOUT:     return "CMD_TIMEOUT";
            case Status::CMD_ERROR:       return "CMD_ERROR";
            case Status::DATA_TIMEOUT:    return "DATA_TIMEOUT";
            case Status::DATA_ERROR:      return "DATA_ERROR";
            case Status::CARD_ERROR:      return "CARD_ERROR";
            default:                      return "?";
        }
    }

    const char* type_name(const CardType t) noexcept
    {
        switch (t)
        {
            case CardType::SD_V1: return "SD 1.x";
            case CardType::SD_V2: return "SD 2.0 standard capacity";
            case CardType::SD_HC: return "SDHC/SDXC";
            case CardType::MMC:   return "MMC";
            default:              return "unknown";
        }
    }

    /*  Dump 'len' bytes as the usual offset / hex / ascii listing. */
    void hexdump(const uint8_t* data, const uint32_t len) noexcept
    {
        char line[80];

        for (uint32_t off = 0; off < len; off += 16u)
        {
            int n = snprintf(line, sizeof(line), "%04lx  ", static_cast<unsigned long>(off));

            for (uint32_t i = 0; i < 16u; ++i)
            {
                n += snprintf(line + n, sizeof(line) - static_cast<size_t>(n), "%02x ",
                              data[off + i]);
            }

            n += snprintf(line + n, sizeof(line) - static_cast<size_t>(n), " ");

            for (uint32_t i = 0; i < 16u; ++i)
            {
                const uint8_t c = data[off + i];
                n += snprintf(line + n, sizeof(line) - static_cast<size_t>(n), "%c",
                              ((c >= 0x20u) && (c < 0x7Fu)) ? static_cast<char>(c) : '.');
            }

            LOG_I(TAG, "%s", line);
        }
    }

    /*  Fill the buffer with a pattern that is different for every block and every run. */
    void make_pattern(uint8_t* buf, const uint32_t blocks, const uint32_t seed) noexcept
    {
        for (uint32_t b = 0; b < blocks; ++b)
        {
            uint8_t* const block = buf + (b * BLOCK_SIZE);

            for (uint32_t i = 0; i < BLOCK_SIZE; ++i)
            {
                block[i] = static_cast<uint8_t>((i + (b * 7u) + seed) & 0xFFu);
            }
        }
    }

    [[nodiscard]] bool compare(const uint8_t* a, const uint8_t* b, const uint32_t len) noexcept
    {
        for (uint32_t i = 0; i < len; ++i)
        {
            if (a[i] != b[i])
            {
                LOG_E(TAG, "mismatch at byte %lu: expected 0x%02x, read 0x%02x",
                      static_cast<unsigned long>(i), a[i], b[i]);
                return false;
            }
        }

        return true;
    }

    /*  Read the given run of blocks, overwrite it with a pattern, verify the pattern, then put the
     *  original content back and verify that too.
     */
    [[nodiscard]] bool write_verify_restore(Card& card, const uint32_t start, const uint32_t blocks,
                                            const uint32_t seed) noexcept
    {
        const uint32_t len = blocks * BLOCK_SIZE;

        LOG_I(TAG, "--- write test: %lu block(s) at LBA %lu ---",
              static_cast<unsigned long>(blocks), static_cast<unsigned long>(start));

        Status st = card.read_blocks(start, g_original, blocks);
        if (st != Status::OK)
        {
            LOG_E(TAG, "saving the original content failed: %s", status_name(st));
            return false;
        }

        make_pattern(g_pattern, blocks, seed);

        st = card.write_blocks(start, g_pattern, blocks);
        if (st != Status::OK)
        {
            LOG_E(TAG, "write failed: %s", status_name(st));
            return false;
        }

        st = card.read_blocks(start, g_readback, blocks);
        if (st != Status::OK)
        {
            LOG_E(TAG, "read back failed: %s", status_name(st));
            return false;
        }

        if (!compare(g_pattern, g_readback, len))
        {
            LOG_E(TAG, "the pattern did not survive the round trip");
            /* Still try to put the original content back before giving up */
            (void)card.write_blocks(start, g_original, blocks);
            return false;
        }

        LOG_I(TAG, "pattern verified over %lu bytes", static_cast<unsigned long>(len));

        st = card.write_blocks(start, g_original, blocks);
        if (st != Status::OK)
        {
            LOG_E(TAG, "restoring the original content failed: %s", status_name(st));
            return false;
        }

        st = card.read_blocks(start, g_readback, blocks);
        if (st != Status::OK)
        {
            LOG_E(TAG, "reading the restored content failed: %s", status_name(st));
            return false;
        }

        if (!compare(g_original, g_readback, len))
        {
            LOG_E(TAG, "the original content was not restored");
            return false;
        }

        LOG_I(TAG, "original content restored and verified");
        return true;
    }
}

bool sd_blocks_test(void)
{
    LOG_I(TAG, "bringing MMC0 up at %lu Hz for card identification",
          static_cast<unsigned long>(INIT_FREQ_HZ));

    Config cfg;
    cfg.bus_voltage  = REGS::MMCHS::VS_3_0V;
    cfg.bus_width    = REGS::MMCHS::WIDTH_1BIT;   // the card is identified on a single data line
    cfg.bus_freq_hz  = INIT_FREQ_HZ;
    cfg.data_timeout = REGS::MMCHS::DTO_TCF_2_27;

    if (!g_mmc0.init(cfg))
    {
        LOG_E(TAG, "controller initialization failed");
        return false;
    }

    /*  Card detect is muxed onto SPI0_CS1 on the BeagleBone Black. Its polarity depends on the
     *  slot, so the state is only reported and never used to refuse the card.
     */
    LOG_I(TAG, "card detect line reads '%s', write protect reads '%s'",
          g_mmc0.is_card_inserted() ? "inserted" : "empty",
          g_mmc0.is_card_write_protected() ? "protected" : "writable");

    if (!g_mmc0.send_init_stream())
    {
        LOG_E(TAG, "the 80-clock initialization stream did not complete");
        return false;
    }

    Card card(g_mmc0);

    if (!card.init(/* wide */ true, TRANSFER_FREQ_HZ))
    {
        LOG_E(TAG, "no card answered the identification sequence");
        return false;
    }

    char name[6];
    card.product_name(name);

    LOG_I(TAG, "card identified: %s, product '%s', RCA 0x%04x",
          type_name(card.type()), name, card.rca());
    LOG_I(TAG, "capacity: %lu blocks of %lu bytes (%lu MiB)",
          static_cast<unsigned long>(card.block_count()),
          static_cast<unsigned long>(BLOCK_SIZE),
          static_cast<unsigned long>(card.capacity_bytes() >> 20));
    LOG_I(TAG, "bus: 4-bit, %lu Hz", static_cast<unsigned long>(TRANSFER_FREQ_HZ));

    /*  --- read test: the very first sector of the card --- */
    LOG_I(TAG, "--- read test: LBA 0 ---");

    Status st = card.read_blocks(0u, g_readback, 1u);
    if (st != Status::OK)
    {
        LOG_E(TAG, "reading LBA 0 failed: %s", status_name(st));
        return false;
    }

    hexdump(g_readback, 64u);

    if ((g_readback[510] == 0x55u) && (g_readback[511] == 0xAAu))
        LOG_I(TAG, "LBA 0 carries the 0x55AA boot signature: this looks like a partitioned card");
    else
        LOG_W(TAG, "LBA 0 has no 0x55AA signature (raw or unformatted card)");

    /*  --- read test: a run of blocks --- */
    LOG_I(TAG, "--- read test: %lu blocks at LBA 0 ---", static_cast<unsigned long>(MULTI_BLOCKS));

    st = card.read_blocks(0u, g_readback, MULTI_BLOCKS);
    if (st != Status::OK)
    {
        LOG_E(TAG, "multi block read failed: %s", status_name(st));
        return false;
    }

    LOG_I(TAG, "multi block read of %lu bytes succeeded",
          static_cast<unsigned long>(MULTI_BLOCKS * BLOCK_SIZE));

    /*  --- write tests on the scratch area --- */
    if (SCRATCH_BLOCK + MULTI_BLOCKS > card.block_count())
    {
        LOG_W(TAG, "the card is too small for the scratch area at LBA %lu, skipping the write test",
              static_cast<unsigned long>(SCRATCH_BLOCK));
        return true;
    }

    if (!write_verify_restore(card, SCRATCH_BLOCK, 1u, 0x11u))
        return false;

    if (!write_verify_restore(card, SCRATCH_BLOCK, MULTI_BLOCKS, 0x5Au))
        return false;

    /*  --- unaligned buffer round, the path FatFs exercises all the time --- */
    LOG_I(TAG, "--- unaligned buffer read at LBA 0 ---");

    st = card.read_blocks(0u, g_readback + 1u, 1u);
    if (st != Status::OK)
    {
        LOG_E(TAG, "unaligned read failed: %s", status_name(st));
        return false;
    }

    LOG_I(TAG, "unaligned read succeeded");

    return true;
}
