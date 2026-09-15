#include "hal/MMCSD/MMCSD.hpp"

namespace HAL::MMCSD
{
    using namespace REGS::MMCHS;

    namespace
    {
        /*  Field masks of the registers this layer touches through whole-register accesses.
         *  The MMC/SD registers only tolerate 32-bit accesses (TRM 18.5), so read-modify-write on
         *  'reg' is used everywhere instead of writes through the bitfields.
         */
        constexpr uint32_t SYSCONFIG_SOFTRESET = BIT(1);
        constexpr uint32_t SYSSTATUS_RESETDONE = BIT(0);

        /*  SD_SYSCONFIG fields that exist on AM335x: AUTOIDLE, ENAWAKEUP, SIDLEMODE and CLOCKACTIVITY.
         *  Bits 13:12 are reserved on this device - there is no STANDBYMODE field here.
         */
        constexpr uint32_t SYSCONFIG_MASK = BIT(0) | BIT(2) | (0x3u << 3) | (0x3u << 8);

        constexpr uint32_t CON_DW8  = BIT(5);
        constexpr uint32_t CON_INIT = BIT(1);

        constexpr uint32_t HCTL_DTW  = BIT(1);
        constexpr uint32_t HCTL_HSPE = BIT(2);
        constexpr uint32_t HCTL_SDBP = BIT(8);
        constexpr uint32_t HCTL_SDVS = 0x7u << 9;

        constexpr uint32_t SYSCTL_ICE        = BIT(0);
        constexpr uint32_t SYSCTL_ICS        = BIT(1);
        constexpr uint32_t SYSCTL_CEN        = BIT(2);
        constexpr uint32_t SYSCTL_CLKD       = 0x3FFu << 6;
        constexpr uint32_t SYSCTL_CLKD_SHIFT = 6u;
        constexpr uint32_t SYSCTL_DTO        = 0xFu << 16;

        constexpr uint32_t BLK_BLEN       = 0xFFFu;
        constexpr uint32_t BLK_NBLK       = 0xFFFFu << 16;
        constexpr uint32_t BLK_NBLK_SHIFT = 16u;

        constexpr uint32_t CMD_DE         = BIT(0);
        constexpr uint32_t CMD_BCE        = BIT(1);
        constexpr uint32_t CMD_ACEN       = BIT(2);
        constexpr uint32_t CMD_DDIR_READ  = BIT(4);
        constexpr uint32_t CMD_MSBS       = BIT(5);
        constexpr uint32_t CMD_RSP_SHIFT  = 16u;
        constexpr uint32_t CMD_CCCE       = BIT(19);
        constexpr uint32_t CMD_CICE       = BIT(20);
        constexpr uint32_t CMD_DP         = BIT(21);
        constexpr uint32_t CMD_INDX_SHIFT = 24u;

        constexpr uint32_t CAPA_VOLT_MASK = CAPA_VOLT_1V8 | CAPA_VOLT_3V0 | CAPA_VOLT_3V3;
        constexpr uint32_t CAPA_HSS       = BIT(21);

        constexpr uint32_t PSTATE_CINS = BIT(16);
        constexpr uint32_t PSTATE_WP   = BIT(19);

        constexpr uint32_t CLKD_MIN = 2u;
        constexpr uint32_t CLKD_MAX = 1023u;

        /*  Error groups used to turn SD_STAT into a Status. */
        constexpr uint32_t CMD_ERR_MASK  = EVENT_ERR_CMD_CRC | EVENT_ERR_CMD_EB | EVENT_ERR_CMD_INDX;
        constexpr uint32_t DATA_ERR_MASK = EVENT_ERR_DATA_CRC | EVENT_ERR_DATA_EB;
    }

    /**
     * @brief   Soft reset the whole MMC/SD controller through SD_SYSCONFIG[1] SOFTRESET
     *
     * @return  true on reset success, false on timeout
     **/
    bool mmcsd_base::soft_reset() noexcept
    {
        m_instance.SYSCONFIG.reg |= SYSCONFIG_SOFTRESET;

        uint32_t timeout = RESET_RETRY_MAX;
        while ((m_instance.SYSSTATUS.reg & SYSSTATUS_RESETDONE) != SYSSTATUS_RESETDONE)
        {
            if (timeout-- == 0)
                return false;
        }

        return true;
    }

    /**
     * @brief   Soft reset the command and/or data lines of the controller
     *
     * @param   flag    Combination of REGS::MMCHS::e_LINE_RESET
     *
     * @note    TRM 18.5.1.18 mandates the (a)(b)(c) sequence: set the bit to 1, poll for 1 to see the
     *          reset start, then poll for 0 to see it complete. The controller can complete a short
     *          reset before the first read, so step (b) only bounds the wait and never fails the call
     *          on its own; step (c) is the one that decides.
     *
     * @return  true on reset success, false on timeout
     **/
    bool mmcsd_base::lines_reset(const uint32_t flag) noexcept
    {
        /* (a) start the reset */
        m_instance.SYSCTL.reg |= flag;

        /* (b) wait for the reset to be seen as started */
        uint32_t timeout = RESET_START_RETRY_MAX;
        while (((m_instance.SYSCTL.reg & flag) != flag) && (timeout-- != 0))
        {
            /* busy wait */
        }

        /* (c) wait for the reset to complete */
        timeout = RESET_RETRY_MAX;
        while ((m_instance.SYSCTL.reg & flag) != 0)
        {
            if (timeout-- == 0)
                return false;
        }

        return true;
    }

    /**
     * @brief   Configure the idle, clock activity and wakeup modes of the controller
     *
     * @param   config  Combination of the SD_SYSCONFIG fields, already shifted to their bit positions
     *
     * @note    The AM335x MMCHS has no STANDBYMODE field - SD_SYSCONFIG bits 13:12 are reserved.
     **/
    void mmcsd_base::system_config(const uint32_t config) noexcept
    {
        m_instance.SYSCONFIG.reg = (m_instance.SYSCONFIG.reg & ~SYSCONFIG_MASK) | (config & SYSCONFIG_MASK);
    }

    /**
     * @brief   Tell whether a card is inserted, as seen by the debounced card detect input
     *
     * @note    Only meaningful when the instance has a dedicated SDCD pin routed to it, and when
     *          SD_CON[7] CDP matches the polarity of the board.
     **/
    bool mmcsd_base::is_card_inserted() const noexcept
    {
        return (m_instance.PSTATE.reg & PSTATE_CINS) != 0u;
    }

    /**
     * @brief   Tell whether the card is write protected, as seen by the SDWP input
     **/
    bool mmcsd_base::is_card_write_protected() const noexcept
    {
        return (m_instance.PSTATE.reg & PSTATE_WP) != 0u;
    }

    /**
     * @brief   Tell whether the controller advertises high speed support in SD_CAPA[21] HSS
     **/
    bool mmcsd_base::is_high_speed_supported() const noexcept
    {
        return (m_instance.CAPA.reg & CAPA_HSS) != 0u;
    }

    /**
     * @brief   Advertise the bus voltages the board can supply
     *
     * @param   volt    Combination of REGS::MMCHS::e_CAPA_VOLT
     **/
    void mmcsd_base::supported_volt_set(const uint32_t volt) noexcept
    {
        m_instance.CAPA.reg = (m_instance.CAPA.reg & ~CAPA_VOLT_MASK) | (volt & CAPA_VOLT_MASK);
    }

    /**
     * @brief   Poll SD_SYSCTL[1] ICS for the internal clock to become stable
     *
     * @param   retry   Retry count; zero samples the status once without polling
     **/
    bool mmcsd_base::is_int_clock_stable(uint32_t retry) const noexcept
    {
        uint32_t status = 0;

        do
        {
            status = m_instance.SYSCTL.reg & SYSCTL_ICS;

            if ((status != 0u) || (retry == 0u))
                break;

        } while (retry--);

        return status != 0u;
    }

    /**
     * @brief   Start or stop the internal clock of the controller
     *
     * @param   on  true to let the internal clock oscillate, false to stop it
     *
     * @return  true when the clock reached the requested state
     **/
    bool mmcsd_base::int_clock(const bool on) noexcept
    {
        if (on)
        {
            m_instance.SYSCTL.reg |= SYSCTL_ICE;
            return is_int_clock_stable(CLOCK_RETRY_MAX);
        }

        m_instance.SYSCTL.reg &= ~SYSCTL_ICE;
        return true;
    }

    /**
     * @brief   Gate or ungate the clock delivered to the card (SD_SYSCTL[2] CEN)
     **/
    void mmcsd_base::card_clock_enable(const bool on) noexcept
    {
        if (on)
            m_instance.SYSCTL.reg |= SYSCTL_CEN;
        else
            m_instance.SYSCTL.reg &= ~SYSCTL_CEN;
    }

    /**
     * @brief   Program the output bus frequency
     *
     * @param   freq_in     Reference frequency of the controller, see INPUT_FREQ_HZ
     * @param   freq_out    Requested frequency on the mmc_clk pin
     * @param   bypass      true to leave the reference clock bypassed and program no divider
     *
     * @note    The divider is rounded up, so the programmed frequency never exceeds freq_out.
     *          The clock to the card is gated while CLKD changes, as TRM 18.5.1.18 requires, and
     *          re-enabled once the internal clock is stable again.
     *
     * @return  true when the clock is running at the requested frequency
     **/
    bool mmcsd_base::bus_freq_set(const uint32_t freq_in, const uint32_t freq_out, const bool bypass) noexcept
    {
        /* First enable the internal clocks */
        if (!int_clock(true))
            return false;

        if (bypass)
            return true;

        if (freq_out == 0u)
            return false;

        uint32_t clkd = freq_in / freq_out;

        clkd = (clkd < CLKD_MIN) ? CLKD_MIN : clkd;
        clkd = (clkd > CLKD_MAX) ? CLKD_MAX : clkd;

        /* Round the divider up so that the bus never runs faster than requested */
        while ((freq_in / clkd) > freq_out)
        {
            if (clkd == CLKD_MAX)
                return false;       /* the requested frequency cannot be reached */

            clkd++;
        }

        /* The frequency may only be changed while the card clock is gated */
        card_clock_enable(false);

        m_instance.SYSCTL.reg = (m_instance.SYSCTL.reg & ~SYSCTL_CLKD) | (clkd << SYSCTL_CLKD_SHIFT);

        if (!is_int_clock_stable(CLOCK_RETRY_MAX))
            return false;

        card_clock_enable(true);

        return true;
    }

    /**
     * @brief   Configure the width of the data bus
     *
     * @param   width   REGS::MMCHS::WIDTH_8BIT, WIDTH_4BIT or WIDTH_1BIT
     *
     * @note    SD_CON[5] DW8 overrides SD_HCTL[1] DTW, so DTW is cleared as well when leaving 8-bit mode.
     **/
    void mmcsd_base::bus_width_set(const e_BUS_WIDTH width) noexcept
    {
        switch (width)
        {
            case WIDTH_8BIT:
                m_instance.CON.reg |= CON_DW8;
                break;

            case WIDTH_4BIT:
                m_instance.CON.reg &= ~CON_DW8;
                m_instance.HCTL.reg |= HCTL_DTW;
                break;

            case WIDTH_1BIT:
                m_instance.CON.reg &= ~CON_DW8;
                m_instance.HCTL.reg &= ~HCTL_DTW;
                break;

            default:
                /* unsupported width - leave the controller untouched */
                break;
        }
    }

    /**
     * @brief   Select the bus voltage advertised on SD_HCTL[11:9] SDVS
     **/
    void mmcsd_base::bus_volt_set(const e_SDVS volt) noexcept
    {
        m_instance.HCTL.reg = (m_instance.HCTL.reg & ~HCTL_SDVS) |
                              ((static_cast<uint32_t>(volt) << 9) & HCTL_SDVS);
    }

    /**
     * @brief   Turn the bus power on or off
     *
     * @note    A write to SD_HCTL[8] SDBP has no effect when the selected voltage is not advertised in
     *          SD_CAPA, so supported_volt_set() and bus_volt_set() must come first.
     *
     * @return  true when the power reached the requested state
     **/
    bool mmcsd_base::bus_power(const bool on) noexcept
    {
        if (!on)
        {
            m_instance.HCTL.reg &= ~HCTL_SDBP;
            return true;
        }

        m_instance.HCTL.reg |= HCTL_SDBP;

        uint32_t timeout = POWER_RETRY_MAX;
        while ((m_instance.HCTL.reg & HCTL_SDBP) == 0u)
        {
            if (timeout-- == 0)
                return false;
        }

        return true;
    }

    /**
     * @brief   Select normal or high speed timing (SD_HCTL[2] HSPE)
     **/
    void mmcsd_base::high_speed_enable(const bool on) noexcept
    {
        if (on)
            m_instance.HCTL.reg |= HCTL_HSPE;
        else
            m_instance.HCTL.reg &= ~HCTL_HSPE;
    }

    /**
     * @brief   Program the data/busy timeout counter (SD_SYSCTL[19:16] DTO)
     **/
    void mmcsd_base::data_timeout_set(const e_DTO timeout) noexcept
    {
        m_instance.SYSCTL.reg = (m_instance.SYSCTL.reg & ~SYSCTL_DTO) |
                                ((static_cast<uint32_t>(timeout) << 16) & SYSCTL_DTO);
    }

    /**
     * @brief   Program the transfer block length (SD_BLK[11:0] BLEN)
     **/
    void mmcsd_base::blk_len_set(const uint32_t blklen) noexcept
    {
        m_instance.BLK.reg = (m_instance.BLK.reg & ~BLK_BLEN) | (blklen & BLK_BLEN);
    }

    /**
     * @brief   Send the mandatory 80-clock initialization stream to the card
     *
     * @note    No command is sent and no response is expected; the controller raises command complete
     *          once the sequence is over.
     * @note    TRM 18.5.1.5 asks for a clock divider that makes 80 clock periods longer than 1 ms,
     *          which would mean a bus below 80 kHz. The SD physical layer specification only asks
     *          for at least 74 clocks in the 100..400 kHz window, and both TI's own starterware and
     *          the Linux omap_hsmmc driver send the stream at 400 kHz. INIT_FREQ_HZ follows them.
     *
     * @return  true when the sequence completed
     **/
    bool mmcsd_base::init_stream_send() noexcept
    {
        intr_status_clear(EVENT_ALL_MASK);
        intr_status_enable(EVENT_CMD_COMPLETE);

        m_instance.CON.reg |= CON_INIT;
        m_instance.CMD.reg = 0x00u;

        uint32_t timeout = CMD_RETRY_MAX;
        bool     done    = false;

        while (timeout-- != 0)
        {
            if ((m_instance.STAT.reg & EVENT_CMD_COMPLETE) != 0u)
            {
                done = true;
                break;
            }
        }

        m_instance.CON.reg &= ~CON_INIT;
        intr_status_clear(EVENT_ALL_MASK);

        return done;
    }

    /**
     * @brief   Push a command into the controller without waiting for its completion
     *
     * @param   cmd     Command description, see HAL::MMCSD::Command
     *
     * @note    SD_ARG must be valid before SD_CMD is written, because the write to the most
     *          significant byte of SD_CMD is what actually starts the command.
     **/
    void mmcsd_base::command_send(const Command& cmd) noexcept
    {
        uint32_t word = (static_cast<uint32_t>(cmd.index) << CMD_INDX_SHIFT) |
                        (static_cast<uint32_t>(cmd.rsp_type) << CMD_RSP_SHIFT);

        if (cmd.check_crc && (cmd.rsp_type != RSP_NONE))
            word |= CMD_CCCE;

        if (cmd.check_index && (cmd.rsp_type != RSP_NONE) && (cmd.rsp_type != RSP_136BIT))
            word |= CMD_CICE;

        if (cmd.data_present)
        {
            word |= CMD_DP;

            if (cmd.read)
                word |= CMD_DDIR_READ;

            if (cmd.nblks > 1u)
            {
                word |= CMD_MSBS | CMD_BCE;

                if (cmd.auto_cmd12)
                    word |= CMD_ACEN;
            }
        }

        if (cmd.dma)
            word |= CMD_DE;

        /* Block count is only consulted for multi block transfers, but keeping it coherent is free */
        m_instance.BLK.reg = (m_instance.BLK.reg & ~BLK_NBLK) |
                             ((cmd.nblks << BLK_NBLK_SHIFT) & BLK_NBLK);

        m_instance.ARG.reg = cmd.arg;
        m_instance.CMD.reg = word;
    }

    /**
     * @brief   Turn the error bits currently set in SD_STAT into a Status and clear them
     *
     * @return  Status::OK when no error bit was set
     **/
    Status mmcsd_base::m_consume_errors() noexcept
    {
        const uint32_t stat = m_instance.STAT.reg;

        if ((stat & EVENT_ERR_MASK) == 0u)
            return Status::OK;

        Status result = Status::CMD_ERROR;

        if ((stat & EVENT_ERR_CMD_TOUT) != 0u)
            result = Status::CMD_TIMEOUT;
        else if ((stat & CMD_ERR_MASK) != 0u)
            result = Status::CMD_ERROR;
        else if ((stat & EVENT_ERR_DATA_TOUT) != 0u)
            result = Status::DATA_TIMEOUT;
        else if ((stat & DATA_ERR_MASK) != 0u)
            result = Status::DATA_ERROR;
        else if ((stat & EVENT_ERR_CARD) != 0u)
            result = Status::CARD_ERROR;

        /* Clear the error bits; ERRI follows them automatically */
        intr_status_clear(stat & EVENT_ERR_MASK);

        /* A command error leaves the command line stuck - the TRM requires a line reset */
        if ((stat & (EVENT_ERR_CMD_TOUT | CMD_ERR_MASK)) != 0u)
            (void)lines_reset(RESET_CMD_LINE);

        if ((stat & (EVENT_ERR_DATA_TOUT | DATA_ERR_MASK)) != 0u)
            (void)lines_reset(RESET_DAT_LINE);

        return result;
    }

    /**
     * @brief   Push a command and wait for the controller to report command complete
     *
     * @param   cmd     Command description
     * @param   retry   Polling budget
     *
     * @return  Status::OK when the card answered, an error code otherwise
     **/
    Status mmcsd_base::send_command(const Command& cmd, uint32_t retry) noexcept
    {
        intr_status_clear(EVENT_ALL_MASK);

        command_send(cmd);

        while (retry-- != 0)
        {
            const uint32_t stat = m_instance.STAT.reg;

            if ((stat & EVENT_ERROR) != 0u)
                return m_consume_errors();

            if ((stat & EVENT_CMD_COMPLETE) != 0u)
            {
                intr_status_clear(EVENT_CMD_COMPLETE);

                /*  A command that answers with busy keeps SD_PSTATE[1] DATI asserted until the card
                 *  releases the DAT0 line; the controller turns that release into transfer complete.
                 *  Commands that carry their own data phase are closed by the caller instead.
                 */
                if ((cmd.rsp_type == RSP_48BIT_BUSY) && !cmd.data_present)
                    return wait_transfer_complete(retry);

                return Status::OK;
            }
        }

        return Status::TIMEOUT;
    }

    /**
     * @brief   Read every response register of the controller
     *
     * @param   rsp     Buffer of at least REGS::MMCHS::RESPONSE_WORDS words
     *
     * @note    A 48-bit response lives in rsp[0] alone, a 136-bit one spans rsp[0..3].
     **/
    void mmcsd_base::response_get(uint32_t* rsp) const noexcept
    {
        if (rsp == nullptr)
            return;

        for (uint32_t i = 0; i < RESPONSE_WORDS; i++)
        {
            rsp[i] = m_instance.RSP(static_cast<uint8_t>(i));
        }
    }

    /**
     * @brief   Read one block out of the controller buffer, in PIO mode
     *
     * @param   dst     Destination buffer, must be 4-byte aligned
     * @param   len     Number of bytes to read, must be a multiple of 4
     *
     * @note    Waits for the buffer read ready event first, so the caller can call this once per block
     *          of a multi block transfer.
     **/
    Status mmcsd_base::data_read(uint8_t* dst, const uint32_t len) noexcept
    {
        if ((dst == nullptr) || ((len % 4u) != 0u))
            return Status::BAD_ARGUMENT;

        uint32_t retry = XFER_RETRY_MAX;

        while (true)
        {
            const uint32_t stat = m_instance.STAT.reg;

            if ((stat & EVENT_ERROR) != 0u)
                return m_consume_errors();

            if ((stat & EVENT_BUF_RD_READY) != 0u)
                break;

            if (retry-- == 0)
                return Status::TIMEOUT;
        }

        intr_status_clear(EVENT_BUF_RD_READY);

        auto* const words = reinterpret_cast<uint32_t*>(dst);

        for (uint32_t i = 0; i < (len / 4u); i++)
        {
            words[i] = m_instance.DATA.reg;
        }

        return Status::OK;
    }

    /**
     * @brief   Write one block into the controller buffer, in PIO mode
     *
     * @param   src     Source buffer, must be 4-byte aligned
     * @param   len     Number of bytes to write, must be a multiple of 4
     *
     * @note    Waits for the buffer write ready event first, so the caller can call this once per block
     *          of a multi block transfer.
     **/
    Status mmcsd_base::data_write(const uint8_t* src, const uint32_t len) noexcept
    {
        if ((src == nullptr) || ((len % 4u) != 0u))
            return Status::BAD_ARGUMENT;

        uint32_t retry = XFER_RETRY_MAX;

        while (true)
        {
            const uint32_t stat = m_instance.STAT.reg;

            if ((stat & EVENT_ERROR) != 0u)
                return m_consume_errors();

            if ((stat & EVENT_BUF_WR_READY) != 0u)
                break;

            if (retry-- == 0)
                return Status::TIMEOUT;
        }

        intr_status_clear(EVENT_BUF_WR_READY);

        const auto* const words = reinterpret_cast<const uint32_t*>(src);

        for (uint32_t i = 0; i < (len / 4u); i++)
        {
            m_instance.DATA.reg = words[i];
        }

        return Status::OK;
    }

    /**
     * @brief   Wait for the controller to report that the whole data transfer is complete
     *
     * @param   retry   Polling budget
     **/
    Status mmcsd_base::wait_transfer_complete(uint32_t retry) noexcept
    {
        while (retry-- != 0)
        {
            const uint32_t stat = m_instance.STAT.reg;

            if ((stat & EVENT_ERROR) != 0u)
                return m_consume_errors();

            if ((stat & EVENT_XFER_COMPLETE) != 0u)
            {
                intr_status_clear(EVENT_XFER_COMPLETE);
                return Status::OK;
            }
        }

        return Status::TIMEOUT;
    }

    /**
     * @brief   Read the selected status bits of SD_STAT
     **/
    uint32_t mmcsd_base::intr_status_get(const uint32_t flag) const noexcept
    {
        return m_instance.STAT.reg & flag;
    }

    /**
     * @brief   Let the selected events set their flag in SD_STAT
     **/
    void mmcsd_base::intr_status_enable(const uint32_t flag) noexcept
    {
        m_instance.IE.reg |= flag;
    }

    /**
     * @brief   Stop the selected events from setting their flag in SD_STAT
     **/
    void mmcsd_base::intr_status_disable(const uint32_t flag) noexcept
    {
        m_instance.IE.reg &= ~flag;
    }

    /**
     * @brief   Clear the selected status bits of SD_STAT
     **/
    void mmcsd_base::intr_status_clear(const uint32_t flag) noexcept
    {
        m_instance.STAT.reg = flag;
    }

    /**
     * @brief   Route the selected events to the module interrupt line
     *
     * @note    SD_ISE only routes an event that SD_IE already lets reach SD_STAT, so both are programmed.
     **/
    void mmcsd_base::intr_enable(const uint32_t flag) noexcept
    {
        m_instance.ISE.reg |= flag;
        intr_status_enable(flag);
    }

    /**
     * @brief   Stop routing the selected events to the module interrupt line
     **/
    void mmcsd_base::intr_disable(const uint32_t flag) noexcept
    {
        m_instance.ISE.reg &= ~flag;
    }

    /**
     * @brief   Capture the register context of the controller
     **/
    void mmcsd_base::context_save(Context& ctx) const noexcept
    {
        ctx.capa      = m_instance.CAPA.reg;
        ctx.sysconfig = m_instance.SYSCONFIG.reg;
        ctx.con       = m_instance.CON.reg;
        ctx.sysctl    = m_instance.SYSCTL.reg;
        ctx.pstate    = m_instance.PSTATE.reg;
        ctx.hctl      = m_instance.HCTL.reg;
    }

    /**
     * @brief   Restore a register context previously captured by context_save()
     *
     * @note    SD_PSTATE is a read-only status register and is therefore not restored; the saved copy
     *          is kept in the context for diagnostics only.
     **/
    void mmcsd_base::context_restore(const Context& ctx) noexcept
    {
        m_instance.SYSCONFIG.reg = ctx.sysconfig;
        m_instance.SYSCTL.reg    = ctx.sysctl;
        m_instance.CAPA.reg      = ctx.capa;
        m_instance.CON.reg       = ctx.con;
        m_instance.HCTL.reg      = ctx.hctl;
    }

    /**
     * @brief   Enable the functional and interface clocks of MMC0
     **/
    void mmcsd0_t::run_clocks() noexcept
    {
        using namespace REGS::PRCM;

        auto& per = *AM335x_CM_PER;

        per.MMC0_CLKCTRL.b.MODULEMODE = MODULEMODE_ENABLE;
        while (per.MMC0_CLKCTRL.b.MODULEMODE != MODULEMODE_ENABLE) {}
        while (per.MMC0_CLKCTRL.b.IDLEST != IDLEST_FUNC) {}

        per.L3_CLKCTRL.b.MODULEMODE = MODULEMODE_ENABLE;
        while (per.L3_CLKCTRL.b.IDLEST != IDLEST_FUNC) {}
    }

    /**
     * @brief   Mux the BeagleBone Black microSD pads to the MMC0 controller
     *
     * @note    The four data lines, the command line and the card detect input need their internal
     *          pull-ups, so that an empty slot and an idle bus read as high. The clock line is driven
     *          by the controller and needs none.
     **/
    void mmcsd0_t::init_pins() noexcept
    {
        using namespace HAL::PINS;
        using namespace REGS::CONTROL_MODULE;

        /*  The pads are owned here rather than by the board file: only their control-module mux is
         *  touched, never their GPIO data path, so no GPIO clock and no board knowledge is needed.
         */
        static MMC0_DAT3 mmc0_dat3{REGS::GPIO::AM335x_GPIO_2};
        static MMC0_DAT2 mmc0_dat2{REGS::GPIO::AM335x_GPIO_2};
        static MMC0_DAT1 mmc0_dat1{REGS::GPIO::AM335x_GPIO_2};
        static MMC0_DAT0 mmc0_dat0{REGS::GPIO::AM335x_GPIO_2};
        static MMC0_CLK  mmc0_clk {REGS::GPIO::AM335x_GPIO_2};
        static MMC0_CMD  mmc0_cmd {REGS::GPIO::AM335x_GPIO_2};
        static SPI0_CS1  spi0_cs1 {REGS::GPIO::AM335x_GPIO_0};

        const auto pull_up_input = [](auto& pad, auto mode)
        {
            pad.sel_pinmode(mode);
            pad.pullup_enable(true);
            pad.sel_pull_type(PULL_UP);
            pad.sel_rxactive(INPUT_ENABLE);
            pad.sel_slewrate(FAST);
        };

        pull_up_input(mmc0_dat3, e_MMC0_DAT3::mmc0_dat3);
        pull_up_input(mmc0_dat2, e_MMC0_DAT2::mmc0_dat2);
        pull_up_input(mmc0_dat1, e_MMC0_DAT1::mmc0_dat1);
        pull_up_input(mmc0_dat0, e_MMC0_DAT0::mmc0_dat0);
        pull_up_input(mmc0_cmd,  e_MMC0_CMD::mmc0_cmd);

        /* The clock is an output driven by the controller: no pull, but the input buffer stays enabled
         * because the controller samples its own clock back. */
        mmc0_clk.sel_pinmode(e_MMC0_CLK::mmc0_clk);
        mmc0_clk.pullup_enable(false);
        mmc0_clk.sel_rxactive(INPUT_ENABLE);
        mmc0_clk.sel_slewrate(FAST);

        /* Card detect of the microSD slot is muxed onto SPI0_CS1 on the BeagleBone Black */
        pull_up_input(spi0_cs1, e_SPI0_CS1::mmc0_sdcd);
    }

}   // namespace HAL::MMCSD
