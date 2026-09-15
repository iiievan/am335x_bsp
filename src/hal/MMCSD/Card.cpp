#include "hal/MMCSD/Card.hpp"

namespace HAL::MMCSD
{
    using namespace REGS::MMCHS;

    namespace
    {
        /*  Operating condition register value the host asks for: the whole 2.7 V .. 3.6 V window. */
        constexpr uint32_t OCR_VOLTAGE_WINDOW = 0x00FF8000u;
        constexpr uint32_t OCR_HCS            = BIT(30);    // host supports high capacity
        constexpr uint32_t OCR_CCS            = BIT(30);    // card is high capacity (in the answer)
        constexpr uint32_t OCR_BUSY           = BIT(31);    // cleared while the card is still powering up

        /*  Check pattern and voltage code carried by CMD8 / answered in R7. */
        constexpr uint32_t IF_COND_VOLT_2V7_3V6 = 0x1u << 8;
        constexpr uint32_t IF_COND_PATTERN      = 0xAAu;

        /*  Card status bits of an R1 response. */
        constexpr uint32_t R1_ERROR_MASK     = 0xFDF98000u;   // every error bit of the card status
        constexpr uint32_t R1_CURRENT_STATE  = 0xFu << 9;
        constexpr uint32_t R1_STATE_SHIFT    = 9u;
        constexpr uint32_t R1_READY_FOR_DATA = BIT(8);

        constexpr uint32_t CARD_STATE_PRG = 7u;

        /*  ACMD41 may take up to one second; the loop below is bounded instead by iterations. */
        constexpr uint32_t OP_COND_RETRY_MAX = 0x4000u;

        /*  Argument of ACMD6 selecting the 4-bit bus. */
        constexpr uint32_t BUS_WIDTH_4BIT_ARG = 0x2u;

        /*  Crude busy wait used between the polling rounds of ACMD41.
         *  This layer owns no timer, so the delay is expressed in loop iterations.
         */
        void short_delay() noexcept
        {
            for (volatile uint32_t i = 0; i < 2000u; ++i)
            {
                __asm volatile("nop");
            }
        }

        [[nodiscard]] bool is_word_aligned(const void* p) noexcept
        {
            return (reinterpret_cast<uintptr_t>(p) & 0x3u) == 0u;
        }

        void copy_block(uint8_t* dst, const uint8_t* src) noexcept
        {
            for (uint32_t i = 0; i < BLOCK_SIZE; ++i)
            {
                dst[i] = src[i];
            }
        }
    }

    /**
     * @brief   CMD55 (APP_CMD) - announce that the next command is an application command
     **/
    Status Card::m_app_command(const uint16_t rca) noexcept
    {
        return m_host.send_command(
            Command::simple(CMD55_ACMD55, static_cast<uint32_t>(rca) << 16, RSP_48BIT));
    }

    /**
     * @brief   CMD0 - put every card on the bus into the idle state
     **/
    bool Card::m_go_idle() noexcept
    {
        return m_host.send_command(Command::simple(CMD0_ACMD0, 0u, RSP_NONE)) == Status::OK;
    }

    /**
     * @brief   CMD8 - check the interface condition and detect a version 2.0 or later card
     *
     * @param   v2  Set to true when the card echoed the check pattern, i.e. it is an SD 2.0+ card
     *
     * @note    A card that does not answer CMD8 is either an SD 1.x card or an MMC; the command
     *          timeout is expected there and is not treated as a failure.
     **/
    bool Card::m_send_if_cond(bool& v2) noexcept
    {
        v2 = false;

        const Status st = m_host.send_command(
            Command::simple(CMD8_ACMD8, IF_COND_VOLT_2V7_3V6 | IF_COND_PATTERN, RSP_48BIT));

        if (st == Status::CMD_TIMEOUT)
            return true;        /* legal: SD 1.x cards simply do not implement CMD8 */

        if (st != Status::OK)
            return false;

        const uint32_t rsp = m_host.response_get_r1();

        /* The card must echo both the voltage code and the check pattern */
        if ((rsp & 0xFFFu) != (IF_COND_VOLT_2V7_3V6 | IF_COND_PATTERN))
            return false;

        v2 = true;
        return true;
    }

    /**
     * @brief   ACMD41 - negotiate the operating condition and wait for the card to leave the busy state
     *
     * @param   v2  true when CMD8 succeeded, which allows asking for high capacity support
     *
     * @note    ACMD41 answers with R3, which carries neither a CRC nor a command index, so both
     *          checks have to be switched off or the controller would flag every answer as an error.
     **/
    bool Card::m_send_op_cond(const bool v2) noexcept
    {
        const Command cmd = Command::unchecked(CMD41_ACMD41,
                                               OCR_VOLTAGE_WINDOW | (v2 ? OCR_HCS : 0u),
                                               RSP_48BIT);

        for (uint32_t retry = OP_COND_RETRY_MAX; retry != 0u; --retry)
        {
            if (m_app_command(0u) != Status::OK)
                return false;       /* CMD55 refused: not an SD card */

            if (m_host.send_command(cmd) != Status::OK)
                return false;

            const uint32_t ocr = m_host.response_get_r1();

            if ((ocr & OCR_BUSY) != 0u)
            {
                m_type = ((ocr & OCR_CCS) != 0u) ? CardType::SD_HC
                                                 : (v2 ? CardType::SD_V2 : CardType::SD_V1);
                return true;
            }

            short_delay();
        }

        return false;
    }

    /**
     * @brief   CMD2 then CMD3 - fetch the card identification and get a relative card address
     **/
    bool Card::m_read_cid_and_rca() noexcept
    {
        if (m_host.send_command(Command::simple(CMD2_ACMD2, 0u, RSP_136BIT)) != Status::OK)
            return false;

        m_host.response_get(m_cid);

        if (m_host.send_command(Command::simple(CMD3_ACMD3, 0u, RSP_48BIT)) != Status::OK)
            return false;

        /* R6 carries the published RCA in the upper half of the response argument */
        m_rca = static_cast<uint16_t>((m_host.response_get_r1() >> 16) & 0xFFFFu);

        return m_rca != 0u;
    }

    /**
     * @brief   CMD9 - fetch the card specific data of the addressed card
     **/
    bool Card::m_read_csd() noexcept
    {
        const Command cmd = Command::simple(CMD9_ACMD9,
                                            static_cast<uint32_t>(m_rca) << 16,
                                            RSP_136BIT);

        if (m_host.send_command(cmd) != Status::OK)
            return false;

        m_host.response_get(m_csd);
        m_decode_csd();

        return m_block_count != 0u;
    }

    /**
     * @brief   Work out the number of 512-byte blocks of the card from its CSD
     *
     * @note    For a 136-bit response the controller drops the 8 trailing CRC/end bits, so the CSD
     *          bit b of the specification sits at bit (b - 8) of the response registers.
     *          CSD version 1.0 encodes the capacity as (C_SIZE + 1) * 2^(C_SIZE_MULT + 2) blocks of
     *          2^READ_BL_LEN bytes; version 2.0 encodes it directly as (C_SIZE + 1) * 512 KByte.
     **/
    void Card::m_decode_csd() noexcept
    {
        const uint32_t csd_structure = (m_csd[3] >> 22) & 0x3u;   // CSD[127:126]

        if (csd_structure == 0x1u)
        {
            /* CSD version 2.0: C_SIZE is CSD[69:48], that is response bits [61:40] */
            const uint32_t c_size = (m_csd[1] >> 8) & 0x3FFFFFu;

            m_block_count = (c_size + 1u) * 1024u;
        }
        else
        {
            /* CSD version 1.0 */
            const uint32_t c_size      = ((m_csd[2] & 0x3u) << 10) | ((m_csd[1] >> 22) & 0x3FFu);  // CSD[73:62]
            const uint32_t c_size_mult = (m_csd[1] >> 7) & 0x7u;                                   // CSD[49:47]
            const uint32_t read_bl_len = (m_csd[2] >> 8) & 0xFu;                                   // CSD[83:80]

            /* capacity = (C_SIZE + 1) << (C_SIZE_MULT + 2 + READ_BL_LEN) bytes */
            const uint32_t mult_shift = c_size_mult + 2u + read_bl_len;

            /* Convert straight to 512-byte blocks, guarding the shift against a malformed CSD */
            if ((mult_shift >= 9u) && (mult_shift < 41u))
                m_block_count = (c_size + 1u) << (mult_shift - 9u);
            else
                m_block_count = 0u;
        }
    }

    /**
     * @brief   CMD7 - move the addressed card into the transfer state
     **/
    bool Card::m_select() noexcept
    {
        const Command cmd = Command::simple(CMD7_ACMD7,
                                            static_cast<uint32_t>(m_rca) << 16,
                                            RSP_48BIT_BUSY);

        return m_host.send_command(cmd) == Status::OK;
    }

    /**
     * @brief   ACMD6 - switch the card and then the controller to the 4-bit bus
     **/
    bool Card::m_set_bus_width_4() noexcept
    {
        if (m_app_command(m_rca) != Status::OK)
            return false;

        if (m_host.send_command(Command::simple(CMD6_ACMD6, BUS_WIDTH_4BIT_ARG, RSP_48BIT)) != Status::OK)
            return false;

        /* The card switched first; only now may the controller follow */
        m_host.bus_width_set(WIDTH_4BIT);

        return true;
    }

    /**
     * @brief   Run the whole card identification sequence
     *
     * @param   wide            true to switch to the 4-bit bus once the card is selected
     * @param   transfer_freq   bus frequency used once the card is in the transfer state
     *
     * @note    The controller must already be initialized and clocked at the identification
     *          frequency, and the 80-clock initialization stream must already have been sent.
     **/
    bool Card::init(const bool wide, const uint32_t transfer_freq) noexcept
    {
        m_type        = CardType::UNKNOWN;
        m_rca         = 0u;
        m_block_count = 0u;
        m_initialized = false;

        if (!m_go_idle())
            return false;

        bool v2 = false;
        if (!m_send_if_cond(v2))
            return false;

        if (!m_send_op_cond(v2))
            return false;

        if (!m_read_cid_and_rca())
            return false;

        /*  The CSD has to be read while the card is in the stand-by state, that is before CMD7. */
        if (!m_read_csd())
            return false;

        if (!m_select())
            return false;

        if (wide && !m_set_bus_width_4())
            return false;

        /*  Every SD card works with 512-byte blocks once selected. High capacity cards ignore
         *  CMD16, but sending it keeps the standard capacity ones in a known state.
         */
        if (m_host.send_command(Command::simple(CMD16_ACMD16, BLOCK_SIZE, RSP_48BIT)) != Status::OK)
            return false;

        m_host.blk_len_set(BLOCK_SIZE);

        /*  The identification phase is over: the bus may now run at full speed. */
        if (!m_host.bus_freq_set(INPUT_FREQ_HZ, transfer_freq))
            return false;

        m_initialized = true;
        return true;
    }

    /**
     * @brief   CMD13 - poll the card status until it leaves the programming state
     **/
    Status Card::wait_ready(uint32_t retry) noexcept
    {
        if (m_rca == 0u)
            return Status::NOT_INITIALIZED;

        const Command cmd = Command::simple(CMD13_ACMD13,
                                            static_cast<uint32_t>(m_rca) << 16,
                                            RSP_48BIT);

        while (retry-- != 0u)
        {
            const Status st = m_host.send_command(cmd);

            if (st != Status::OK)
                return st;

            const uint32_t status = m_host.response_get_r1();

            if ((status & R1_ERROR_MASK) != 0u)
                return Status::CARD_ERROR;

            const uint32_t state = (status & R1_CURRENT_STATE) >> R1_STATE_SHIFT;

            if ((state != CARD_STATE_PRG) && ((status & R1_READY_FOR_DATA) != 0u))
                return Status::OK;
        }

        return Status::TIMEOUT;
    }

    /**
     * @brief   Read a run of 512-byte blocks from the card
     *
     * @param   start       Index of the first block
     * @param   dst         Destination buffer of nblocks * BLOCK_SIZE bytes, any alignment
     * @param   nblocks     Number of blocks to read
     *
     * @note    A multi block read is closed by the controller itself through Auto CMD12.
     **/
    Status Card::read_blocks(const uint32_t start, uint8_t* dst, const uint32_t nblocks) noexcept
    {
        if (!m_initialized)
            return Status::NOT_INITIALIZED;

        if ((dst == nullptr) || (nblocks == 0u))
            return Status::BAD_ARGUMENT;

        if ((m_block_count != 0u) && ((start > m_block_count) || (nblocks > (m_block_count - start))))
            return Status::BAD_ARGUMENT;

        const Status ready = wait_ready();
        if (ready != Status::OK)
            return ready;

        m_host.blk_len_set(BLOCK_SIZE);

        const bool multi = nblocks > 1u;

        const Status sent = m_host.send_command(
            Command::transfer(multi ? CMD18_ACMD18 : CMD17_ACMD17,
                              m_data_address(start), true, nblocks));

        if (sent != Status::OK)
            return sent;

        const bool aligned = is_word_aligned(dst);

        for (uint32_t i = 0; i < nblocks; ++i)
        {
            uint8_t* const block = dst + (static_cast<uint32_t>(i) * BLOCK_SIZE);
            uint8_t* const sink  = aligned ? block : m_bounce;

            const Status st = m_host.data_read(sink, BLOCK_SIZE);
            if (st != Status::OK)
                return st;

            if (!aligned)
                copy_block(block, m_bounce);
        }

        return m_host.wait_transfer_complete();
    }

    /**
     * @brief   Write a run of 512-byte blocks to the card
     *
     * @param   start       Index of the first block
     * @param   src         Source buffer of nblocks * BLOCK_SIZE bytes, any alignment
     * @param   nblocks     Number of blocks to write
     *
     * @note    A multi block write is closed by the controller itself through Auto CMD12. Transfer
     *          complete only means the data left the host, so the card is polled afterwards until it
     *          has finished programming them.
     **/
    Status Card::write_blocks(const uint32_t start, const uint8_t* src, const uint32_t nblocks) noexcept
    {
        if (!m_initialized)
            return Status::NOT_INITIALIZED;

        if ((src == nullptr) || (nblocks == 0u))
            return Status::BAD_ARGUMENT;

        if ((m_block_count != 0u) && ((start > m_block_count) || (nblocks > (m_block_count - start))))
            return Status::BAD_ARGUMENT;

        const Status ready = wait_ready();
        if (ready != Status::OK)
            return ready;

        m_host.blk_len_set(BLOCK_SIZE);

        const bool multi = nblocks > 1u;

        const Status sent = m_host.send_command(
            Command::transfer(multi ? CMD25_ACMD25 : CMD24_ACMD24,
                              m_data_address(start), false, nblocks));

        if (sent != Status::OK)
            return sent;

        const bool aligned = is_word_aligned(src);

        for (uint32_t i = 0; i < nblocks; ++i)
        {
            const uint8_t* const block = src + (static_cast<uint32_t>(i) * BLOCK_SIZE);
            const uint8_t*       feed  = block;

            if (!aligned)
            {
                copy_block(m_bounce, block);
                feed = m_bounce;
            }

            const Status st = m_host.data_write(feed, BLOCK_SIZE);
            if (st != Status::OK)
                return st;
        }

        const Status done = m_host.wait_transfer_complete();
        if (done != Status::OK)
            return done;

        /*  The data are in the card buffer; wait until they are actually programmed. */
        return wait_ready();
    }

    /**
     * @brief   Extract the 5-character product name of the CID
     *
     * @param   out     Buffer of 6 bytes, filled with the NUL terminated name
     *
     * @note    The product name is CID[103:64]. For a 136-bit response the controller drops the 8
     *          trailing CRC/end bits, so those are response bits [95:56], that is the whole of
     *          m_cid[2] plus the top byte of m_cid[1].
     **/
    void Card::product_name(char out[6]) const noexcept
    {
        if (out == nullptr)
            return;

        out[0] = static_cast<char>((m_cid[2] >> 24) & 0xFFu);
        out[1] = static_cast<char>((m_cid[2] >> 16) & 0xFFu);
        out[2] = static_cast<char>((m_cid[2] >>  8) & 0xFFu);
        out[3] = static_cast<char>((m_cid[2]      ) & 0xFFu);
        out[4] = static_cast<char>((m_cid[1] >> 24) & 0xFFu);
        out[5] = '\0';

        for (uint32_t i = 0; i < 5u; ++i)
        {
            if ((out[i] < 0x20) || (out[i] > 0x7E))
                out[i] = '?';
        }
    }

}   // namespace HAL::MMCSD
