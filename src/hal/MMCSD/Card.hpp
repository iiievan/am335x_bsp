#ifndef HAL_MMCSD_CARD_HPP
#define HAL_MMCSD_CARD_HPP

#include <cstdint>

#include "hal/MMCSD/MMCSD.hpp"

namespace HAL::MMCSD
{
    /*  Every SD card addresses its data in 512-byte units, whatever its CSD says about the
     *  read/write block length of the media itself.
     */
    constexpr uint32_t BLOCK_SIZE = 512u;

    /*  Card families this driver can identify. */
    enum class CardType : uint8_t
    {
        UNKNOWN = 0,
        SD_V1,          // SD 1.x, byte addressed
        SD_V2,          // SD 2.0 standard capacity, byte addressed
        SD_HC,          // SDHC/SDXC, block addressed
        MMC             // recognized but not driven by this layer
    };

    /*  Card identification and data transfer on top of one MMC/SD controller.
     *
     *  The object does not own the controller: it borrows a reference to an already initialized
     *  mmcsd_base, so the same protocol layer can drive MMC0, MMC1 or MMC2 without being a template.
     */
    class Card
    {
    public:
        explicit Card(mmcsd_base& host) noexcept
            : m_host(host)
        {}

        /*  Run the full card identification sequence: idle, interface condition, operating condition,
         *  CID, relative address, CSD, select, 4-bit bus and the final transfer clock.
         *
         *  @param  wide            true to switch the bus to 4 bits once the card is selected
         *  @param  transfer_freq   bus frequency to use once the card is in the transfer state
         *
         *  @return true when the card is selected and ready for read/write
         */
        [[nodiscard]] bool init(bool wide = true, uint32_t transfer_freq = TRANSFER_FREQ_HZ) noexcept;

        /*  Read nblocks of BLOCK_SIZE bytes starting at block 'start'.
         *  'dst' may have any alignment: unaligned buffers go through an internal bounce buffer.
         */
        [[nodiscard]] Status read_blocks(uint32_t start, uint8_t* dst, uint32_t nblocks) noexcept;

        /*  Write nblocks of BLOCK_SIZE bytes starting at block 'start'.
         *  'src' may have any alignment: unaligned buffers go through an internal bounce buffer.
         */
        [[nodiscard]] Status write_blocks(uint32_t start, const uint8_t* src, uint32_t nblocks) noexcept;

        /*  Poll the card status until it leaves the programming state. */
        [[nodiscard]] Status wait_ready(uint32_t retry = 0x200000u) noexcept;

        [[nodiscard]] bool     is_initialized() const noexcept { return m_initialized; }
        [[nodiscard]] CardType type()           const noexcept { return m_type; }
        [[nodiscard]] bool     is_high_capacity() const noexcept { return m_type == CardType::SD_HC; }
        [[nodiscard]] uint16_t rca()            const noexcept { return m_rca; }
        [[nodiscard]] uint32_t block_count()    const noexcept { return m_block_count; }
        [[nodiscard]] uint64_t capacity_bytes() const noexcept
        {
            return static_cast<uint64_t>(m_block_count) * BLOCK_SIZE;
        }

        [[nodiscard]] const uint32_t* raw_cid() const noexcept { return m_cid; }
        [[nodiscard]] const uint32_t* raw_csd() const noexcept { return m_csd; }

        /*  Product name taken from the CID, NUL terminated. */
        void product_name(char out[6]) const noexcept;

        Card(const Card&) = delete;
        Card& operator=(const Card&) = delete;

    private:
        mmcsd_base& m_host;

        CardType m_type{CardType::UNKNOWN};
        uint16_t m_rca{0u};
        uint32_t m_block_count{0u};
        bool     m_initialized{false};

        uint32_t m_cid[REGS::MMCHS::RESPONSE_WORDS]{};
        uint32_t m_csd[REGS::MMCHS::RESPONSE_WORDS]{};

        /*  Bounce buffer used when the caller hands over a buffer the controller cannot address
         *  with 32-bit accesses. One block is enough: the data path is driven block by block.
         */
        alignas(4) uint8_t m_bounce[BLOCK_SIZE]{};

        [[nodiscard]] Status m_app_command(uint16_t rca) noexcept;
        [[nodiscard]] bool   m_go_idle() noexcept;
        [[nodiscard]] bool   m_send_if_cond(bool& v2) noexcept;
        [[nodiscard]] bool   m_send_op_cond(bool v2) noexcept;
        [[nodiscard]] bool   m_read_cid_and_rca() noexcept;
        [[nodiscard]] bool   m_read_csd() noexcept;
        [[nodiscard]] bool   m_select() noexcept;
        [[nodiscard]] bool   m_set_bus_width_4() noexcept;
        void                 m_decode_csd() noexcept;

        /*  Translate a block index into the argument of a read/write command:
         *  high capacity cards are addressed in blocks, the others in bytes.
         */
        [[nodiscard]] uint32_t m_data_address(uint32_t block) const noexcept
        {
            return is_high_capacity() ? block : (block * BLOCK_SIZE);
        }
    };

}   // namespace HAL::MMCSD

#endif //HAL_MMCSD_CARD_HPP
