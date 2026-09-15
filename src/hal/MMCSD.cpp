#include "hal/MMCSD.hpp"
#include "regs/MMCHS.hpp"

namespace HAL::MMCSD
{
    using namespace REGS::MMCHS;

    static AM335x_MMCHS_Type * mmchs = nullptr;

    /*  Bit masks of the fields this layer manipulates through whole-register accesses.
     *  The MMC/SD registers only tolerate 32-bit accesses (TRM 18.5), so read-modify-write on 'reg'
     *  is used everywhere instead of writes through the bitfields.
     */
    namespace
    {
        constexpr uint32_t SYSCONFIG_SOFTRESET = BIT(1);
        constexpr uint32_t SYSSTATUS_RESETDONE = BIT(0);

        /*  SD_SYSCONFIG fields that exist on AM335x: AUTOIDLE, ENAWAKEUP, SIDLEMODE and CLOCKACTIVITY.
         *  Bits 13:12 are reserved on this device - there is no STANDBYMODE field here.
         */
        constexpr uint32_t SYSCONFIG_CONFIG_MASK = BIT(0) | BIT(2) | (0x3u << 3) | (0x3u << 8);

        constexpr uint32_t CON_DW8    = BIT(5);
        constexpr uint32_t CON_INIT   = BIT(1);

        constexpr uint32_t HCTL_DTW       = BIT(1);
        constexpr uint32_t HCTL_SDBP      = BIT(8);
        constexpr uint32_t HCTL_SDVS      = 0x7u << 9;
        constexpr uint32_t HCTL_DTW_SHIFT = 1u;
        constexpr uint32_t HCTL_DTW_1BIT  = 0x0u;
        constexpr uint32_t HCTL_DTW_4BIT  = 0x1u;

        constexpr uint32_t SYSCTL_ICE        = BIT(0);
        constexpr uint32_t SYSCTL_ICS        = BIT(1);
        constexpr uint32_t SYSCTL_CEN        = BIT(2);
        constexpr uint32_t SYSCTL_CLKD       = 0x3FFu << 6;
        constexpr uint32_t SYSCTL_CLKD_SHIFT = 6u;
        constexpr uint32_t SYSCTL_DTO        = 0xFu << 16;

        constexpr uint32_t STAT_CC = BIT(0);
        constexpr uint32_t STAT_TC = BIT(1);

        constexpr uint32_t BLK_BLEN       = 0xFFFu;
        constexpr uint32_t BLK_NBLK       = 0xFFFFu << 16;
        constexpr uint32_t BLK_NBLK_SHIFT = 16u;

        constexpr uint32_t CMD_DE   = BIT(0);
        constexpr uint32_t CMD_BCE  = BIT(1);
        constexpr uint32_t CMD_MSBS = BIT(5);
        constexpr uint32_t CMD_DP   = BIT(21);

        constexpr uint32_t CAPA_VS18      = BIT(26);
        constexpr uint32_t CAPA_VS30      = BIT(25);
        constexpr uint32_t CAPA_VS33      = BIT(24);
        constexpr uint32_t CAPA_HSS       = BIT(21);
        constexpr uint32_t CAPA_HSS_SHIFT = 21u;

        constexpr uint32_t PSTATE_CINS       = BIT(16);
        constexpr uint32_t PSTATE_CINS_SHIFT = 16u;
        constexpr uint32_t PSTATE_WP         = BIT(19);
        constexpr uint32_t PSTATE_WP_SHIFT   = 19u;

        constexpr uint32_t INTCLOCK_ON = SYSCTL_ICE;
        constexpr uint32_t BUS_POWER_ON = HCTL_SDBP;

        constexpr uint32_t CLKD_MIN = 2u;
        constexpr uint32_t CLKD_MAX = 1023u;
    }

    /**
     * @brief   Bind this layer to one MMC/SD controller instance
     *
     * @param   base_addr   Base address of the MMCHS instance, see REGS::MMCHS::AM335x_MMCHS_n
     **/
    void construct_mmcsd(void * base_addr) noexcept
    {
        mmchs = static_cast<AM335x_MMCHS_Type *>(base_addr);
    }

    /**
     * @brief   Tell whether construct_mmcsd() has already bound an instance
     **/
    bool is_constructed() noexcept
    {
        return (mmchs != nullptr);
    }

    /**
     * @brief   Give back the bound controller instance, or nullptr if there is none
     **/
    AM335x_MMCHS_Type * instance() noexcept
    {
        return mmchs;
    }

    /**
     * @brief   Soft reset the MMC/SD controller
     *
     * @return   0   on reset success
     *          -1   on reset fail or when no instance is bound
     **/
    int soft_reset() noexcept
    {
        if (!is_constructed())
            return -1;

        mmchs->SYSCONFIG.reg |= SYSCONFIG_SOFTRESET;

        uint32_t timeout = RESET_RETRY_MAX;
        while ((mmchs->SYSSTATUS.reg & SYSSTATUS_RESETDONE) != SYSSTATUS_RESETDONE)
        {
            if (timeout-- == 0)
                return -1;
        }

        return 0;
    }

    /**
     * @brief   Soft reset the MMC/SD controller lines
     *
     * @param   flag    Reset flags indicating the lines to be reset, see REGS::MMCHS::e_LINE_RESET:
     *                  RESET_DAT_LINE, RESET_CMD_LINE or RESET_ALL
     *
     * @note    TRM 18.5.1.18 mandates the (a)(b)(c) sequence: set the bit to 1, poll for 1 to see the reset
     *          start, then poll for 0 to see it complete. The controller may complete the reset before the
     *          first poll, so step (b) only bounds the wait and never fails the call on its own; step (c)
     *          is the one that decides success.
     *
     * @return   0   on reset success
     *          -1   on reset fail or when no instance is bound
     **/
    int lines_reset(const uint32_t flag) noexcept
    {
        if (!is_constructed())
            return -1;

        /* (a) start the reset */
        mmchs->SYSCTL.reg |= flag;

        /* (b) wait for the reset to be seen as started. The controller can complete a short reset before
         *     the first read, so this window is deliberately small and its expiry is not an error: it only
         *     keeps step (c) from sampling SD_SYSCTL before the reset had a chance to assert. */
        uint32_t timeout = RESET_START_RETRY_MAX;
        while (((mmchs->SYSCTL.reg & flag) != flag) && (timeout-- != 0))
        {
            /* busy wait */
        }

        /* (c) wait for the reset to complete - this is the step that decides success */
        timeout = RESET_RETRY_MAX;
        while ((mmchs->SYSCTL.reg & flag) != 0)
        {
            if (timeout-- == 0)
                return -1;
        }

        return 0;
    }

    /**
     * @brief   Configure the MMC/SD controller idle, clock activity and wakeup modes
     *
     * @param   config  Combination of the SD_SYSCONFIG fields to apply: AUTOIDLE, ENAWAKEUP,
     *                  SIDLEMODE (see e_SIDLEMODE) and CLOCKACTIVITY (see e_CLOCKACTIVITY), already shifted
     *                  to their bit positions.
     *
     * @note    The AM335x MMCHS has no STANDBYMODE field - SD_SYSCONFIG bits 13:12 are reserved.
     **/
    void system_config(const uint32_t config) noexcept
    {
        if (!is_constructed())
            return;

        mmchs->SYSCONFIG.reg = (mmchs->SYSCONFIG.reg & ~SYSCONFIG_CONFIG_MASK) | (config & SYSCONFIG_CONFIG_MASK);
    }

    /**
     * @brief   Configure the MMC/SD bus width
     *
     * @param   width   SD/MMC bus width, see REGS::MMCHS::e_BUS_WIDTH:
     *                  WIDTH_8BIT, WIDTH_4BIT or WIDTH_1BIT
     **/
    void bus_width_set(const uint32_t width) noexcept
    {
        if (!is_constructed())
            return;

        switch (static_cast<e_BUS_WIDTH>(width))
        {
            case WIDTH_8BIT:
                mmchs->CON.reg |= CON_DW8;
                break;

            case WIDTH_4BIT:
                mmchs->CON.reg &= ~CON_DW8;
                mmchs->HCTL.reg = (mmchs->HCTL.reg & ~HCTL_DTW) | (HCTL_DTW_4BIT << HCTL_DTW_SHIFT);
                break;

            case WIDTH_1BIT:
                mmchs->CON.reg &= ~CON_DW8;
                mmchs->HCTL.reg = (mmchs->HCTL.reg & ~HCTL_DTW) | (HCTL_DTW_1BIT << HCTL_DTW_SHIFT);
                break;

            default:
                /* unsupported width - leave the controller untouched */
                break;
        }
    }

    /**
     * @brief   Configure the MMC/SD bus voltage
     *
     * @param   volt    SD/MMC bus voltage, an e_SDVS value already shifted into SD_HCTL[11:9]
     **/
    void bus_volt_set(const uint32_t volt) noexcept
    {
        if (!is_constructed())
            return;

        mmchs->HCTL.reg = (mmchs->HCTL.reg & ~HCTL_SDVS) | (volt & HCTL_SDVS);
    }

    /**
     * @brief   Turn the MMC/SD bus power on / off
     *
     * @param   pwr     Power setting: SD_HCTL[8] SDBP set for power on, cleared for power off
     *
     * @return  0 if the operation succeeded
     *         -1 if the power did not come up, or when no instance is bound
     **/
    int bus_power(const uint32_t pwr) noexcept
    {
        if (!is_constructed())
            return -1;

        mmchs->HCTL.reg = (mmchs->HCTL.reg & ~HCTL_SDBP) | (pwr & HCTL_SDBP);

        if ((pwr & HCTL_SDBP) == BUS_POWER_ON)
        {
            uint32_t timeout = POWER_RETRY_MAX;
            while ((mmchs->HCTL.reg & HCTL_SDBP) == 0)
            {
                if (timeout-- == 0)
                    return -1;
            }
        }

        return 0;
    }

    /**
     * @brief   Turn the internal clock on / off
     *
     * @param   pwr     Clock setting: SD_SYSCTL[0] ICE set to start the internal clock, cleared to stop it
     *
     * @return  0 if the operation succeeded
     *         -1 if the clock did not become stable, or when no instance is bound
     **/
    int int_clock(const uint32_t pwr) noexcept
    {
        if (!is_constructed())
            return -1;

        mmchs->SYSCTL.reg = (mmchs->SYSCTL.reg & ~SYSCTL_ICE) | (pwr & SYSCTL_ICE);

        if ((pwr & SYSCTL_ICE) == INTCLOCK_ON)
        {
            if (is_int_clock_stable(CLOCK_RETRY_MAX) == 0)
                return -1;
        }

        return 0;
    }

    /**
     * @brief   Get the internal clock stable status
     *
     * @param   retry   Retry count used to poll for stability
     *
     * @note    If retry is zero the status is sampled once and not polled.
     *
     * @return  1 if the clock is stable
     *          0 if the clock is not stable, or when no instance is bound
     **/
    uint32_t is_int_clock_stable(uint32_t retry) noexcept
    {
        if (!is_constructed())
            return 0;

        uint32_t status = 0;

        do
        {
            status = (mmchs->SYSCTL.reg & SYSCTL_ICS) >> 1u;

            if ((status == 1u) || (retry == 0u))
                break;

        } while (retry--);

        return status;
    }

    /**
     * @brief   Set the supported voltage list advertised by the controller
     *
     * @param   volt    Supported bus voltages, a combination of SD_CAPA VS18 / VS30 / VS33
     **/
    void supported_volt_set(const uint32_t volt) noexcept
    {
        if (!is_constructed())
            return;

        constexpr uint32_t volt_mask = CAPA_VS18 | CAPA_VS30 | CAPA_VS33;

        mmchs->CAPA.reg = (mmchs->CAPA.reg & ~volt_mask) | (volt & volt_mask);
    }

    /**
     * @brief   Check whether the controller supports high speed
     *
     * @return  0 if high speed is not supported
     *          1 if high speed is supported
     **/
    uint32_t is_hsupported() noexcept
    {
        if (!is_constructed())
            return 0;

        return (mmchs->CAPA.reg & CAPA_HSS) >> CAPA_HSS_SHIFT;
    }

    /**
     * @brief   Set the data timeout value
     *
     * @param   timeout     Timeout code, an e_DTO value already shifted into SD_SYSCTL[19:16].
     *                      The timeout is TCF x 2^(13 + code), so the code covers 2^13 .. 2^27.
     **/
    void data_timeout_set(const uint32_t timeout) noexcept
    {
        if (!is_constructed())
            return;

        mmchs->SYSCTL.reg = (mmchs->SYSCTL.reg & ~SYSCTL_DTO) | (timeout & SYSCTL_DTO);
    }

    /**
     * @brief   Set the output bus frequency
     *
     * @param   freq_in     The input/reference frequency of the controller
     * @param   freq_out    The required output frequency on the bus
     * @param   bypass      Non-zero to keep the reference clock bypassed (no divider programmed)
     *
     * @return   0  on clock enable success
     *          -1  on clock enable fail, on a zero freq_out, or when no instance is bound
     *
     * @note    On success the clock is enabled to the card by the time this function returns.
     **/
    int bus_freq_set(const uint32_t freq_in, const uint32_t freq_out, const uint32_t bypass) noexcept
    {
        if (!is_constructed())
            return -1;

        /* First enable the internal clocks */
        if (int_clock(INTCLOCK_ON) == -1)
            return -1;

        if (bypass == 0)
        {
            if (freq_out == 0)
                return -1;

            /* Calculate and program the divisor */
            uint32_t clkd = freq_in / freq_out;

            clkd = (clkd < CLKD_MIN) ? CLKD_MIN : clkd;
            clkd = (clkd > CLKD_MAX) ? CLKD_MAX : clkd;

            /* Do not cross the required freq */
            while ((freq_in / clkd) > freq_out)
            {
                if (clkd == CLKD_MAX)
                    return -1;      /* the requested frequency cannot be reached */

                clkd++;
            }

            mmchs->SYSCTL.reg = (mmchs->SYSCTL.reg & ~SYSCTL_CLKD) | (clkd << SYSCTL_CLKD_SHIFT);

            /* Wait for the interface clock stabilization */
            if (is_int_clock_stable(CLOCK_RETRY_MAX) == 0)
                return -1;

            /* Enable the clock to the card */
            mmchs->SYSCTL.reg |= SYSCTL_CEN;
        }

        return 0;
    }

    /**
     * @brief   Send the INIT stream to the card
     *
     * @return   0  if the INIT sequence completed
     *          -1  if the INIT sequence did not complete, or when no instance is bound
     **/
    int init_stream_send() noexcept
    {
        if (!is_constructed())
            return -1;

        /* Enable the command completion status to be set */
        intr_status_enable(EVENT_CMD_COMPLETE);

        /* Initiate the INIT command */
        mmchs->CON.reg |= CON_INIT;
        mmchs->CMD.reg = 0x00u;

        const uint32_t status = is_cmd_complete(CMD_RETRY_MAX);

        mmchs->CON.reg &= ~CON_INIT;

        /* Clear all status */
        intr_status_clear(EVENT_ALL_MASK);

        return (status == 1u) ? 0 : -1;
    }

    /**
     * @brief   Enable the controller events that set flags in the status register
     *
     * @param   flag    Combination of REGS::MMCHS::e_EVENT values
     *
     * @note    This only enables the reflection of the events in SD_STAT. To let an event raise a hardware
     *          interrupt request use intr_enable().
     **/
    void intr_status_enable(const uint32_t flag) noexcept
    {
        if (!is_constructed())
            return;

        mmchs->IE.reg |= flag;
    }

    /**
     * @brief   Disable the controller events that set flags in the status register
     *
     * @param   flag    Combination of REGS::MMCHS::e_EVENT values
     **/
    void intr_status_disable(const uint32_t flag) noexcept
    {
        if (!is_constructed())
            return;

        mmchs->IE.reg &= ~flag;
    }

    /**
     * @brief   Enable the controller events that generate a hardware interrupt request
     *
     * @param   flag    Combination of REGS::MMCHS::e_EVENT values
     *
     * @note    SD_ISE only routes an event to the interrupt line once SD_IE lets it reach SD_STAT,
     *          so both registers are programmed here.
     **/
    void intr_enable(const uint32_t flag) noexcept
    {
        if (!is_constructed())
            return;

        mmchs->ISE.reg |= flag;
        intr_status_enable(flag);
    }

    /**
     * @brief   Get the status bits of the controller
     *
     * @param   flag    Combination of REGS::MMCHS::e_EVENT values to look at
     *
     * @return  The requested status flags
     **/
    uint32_t intr_status_get(const uint32_t flag) noexcept
    {
        if (!is_constructed())
            return 0;

        return mmchs->STAT.reg & flag;
    }

    /**
     * @brief   Clear the status bits of the controller
     *
     * @param   flag    Combination of REGS::MMCHS::e_EVENT values to clear
     **/
    void intr_status_clear(const uint32_t flag) noexcept
    {
        if (!is_constructed())
            return;

        mmchs->STAT.reg = flag;
    }

    /**
     * @brief   Check whether the command is complete
     *
     * @param   retry   Retry count used to poll for completion; zero samples the status once
     *
     * @return  1 if the command is complete
     *          0 if the command is not complete, or when no instance is bound
     **/
    uint32_t is_cmd_complete(uint32_t retry) noexcept
    {
        if (!is_constructed())
            return 0;

        uint32_t status = 0;

        do
        {
            status = mmchs->STAT.reg & STAT_CC;

            if ((status != 0u) || (retry == 0u))
                break;

        } while (retry--);

        return (status != 0u) ? 1u : 0u;
    }

    /**
     * @brief   Check whether the transfer is complete
     *
     * @param   retry   Retry count used to poll for completion; zero samples the status once
     *
     * @return  1 if the transfer is complete
     *          0 if the transfer is not complete, or when no instance is bound
     **/
    uint32_t is_xfer_complete(uint32_t retry) noexcept
    {
        if (!is_constructed())
            return 0;

        uint32_t status = 0;

        do
        {
            status = mmchs->STAT.reg & STAT_TC;

            if ((status != 0u) || (retry == 0u))
                break;

        } while (retry--);

        return (status != 0u) ? 1u : 0u;
    }

    /**
     * @brief   Set the block length/size for the data transfer
     *
     * @param   blklen  Block length in bytes, must fit SD_BLK[11:0] BLEN and stay within the limits
     *                  of the controller and of the card (see REGS::MMCHS::BLOCK_LEN_MAX)
     **/
    void blk_len_set(const uint32_t blklen) noexcept
    {
        if (!is_constructed())
            return;

        mmchs->BLK.reg = (mmchs->BLK.reg & ~BLK_BLEN) | (blklen & BLK_BLEN);
    }

    /**
     * @brief   Pass the MMC/SD command to the controller/card
     *
     * @param   cmd     Command word, already built for SD_CMD (index, type, response type, direction)
     * @param   cmdarg  Argument of the command
     * @param   data    Data pointer if this is a data command, nullptr otherwise. Only its nullness is
     *                  used, to decide whether the data-present bits have to be added to the command.
     * @param   nblks   Data length in number of blocks (multiples of BLEN)
     * @param   dmaEn   Non-zero to enable DMA for this transfer
     **/
    void command_send(uint32_t cmd, const uint32_t cmdarg, const void * data, const uint32_t nblks, const uint32_t dmaEn) noexcept
    {
        if (!is_constructed())
            return;

        if (nullptr != data)
            cmd |= (CMD_DP | CMD_MSBS | CMD_BCE);

        if (0 != dmaEn)
            cmd |= CMD_DE;

        /* Set the block information; the block length is programmed separately by blk_len_set() */
        mmchs->BLK.reg = (mmchs->BLK.reg & ~BLK_NBLK) | ((nblks << BLK_NBLK_SHIFT) & BLK_NBLK);

        /* Set the command argument first: SD_ARG must be valid before the write to SD_CMD starts the command */
        mmchs->ARG.reg = cmdarg;
        mmchs->CMD.reg = cmd;
    }

    /**
     * @brief   Get the command response from the controller
     *
     * @param   rsp     Pointer to a buffer of at least REGS::MMCHS::RESPONSE_WORDS 32-bit words
     *
     * @note    All four response registers are returned. It is up to the caller to use only the relevant
     *          part: a 48-bit response lives in rsp[0] alone, a 136-bit one spans rsp[0..3].
     **/
    void response_get(uint32_t * rsp) noexcept
    {
        if (!is_constructed() || (rsp == nullptr))
            return;

        for (uint32_t i = 0; i < RESPONSE_WORDS; i++)
        {
            rsp[i] = mmchs->RSP(static_cast<uint8_t>(i));
        }
    }

    /**
     * @brief   Read the received data out of the controller buffer
     *
     * @param   data    Pointer to the buffer to fill; it must be 4-byte aligned
     * @param   len     Length of the data in bytes, it must be a multiple of 4
     *
     * @note    The data register is read in chunks of 32 bits, as required by the MMC/SD register access
     *          rules, so a trailing partial word is not transferred.
     **/
    void data_get(uint8_t * data, const uint32_t len) noexcept
    {
        if (!is_constructed() || (data == nullptr))
            return;

        uint32_t * const words = reinterpret_cast<uint32_t *>(data);

        for (uint32_t i = 0; i < (len / 4u); i++)
        {
            words[i] = mmchs->DATA.reg;
        }
    }

    /**
     * @brief   Check whether a card is inserted and detected
     *
     * @return  0  if no card is inserted and detected
     *          1  if a card is inserted and detected
     *
     * @note    This is only meaningful when the controller instance has a dedicated card detect pin.
     *          Otherwise the card detection mechanism is application specific.
     **/
    uint32_t is_card_inserted() noexcept
    {
        if (!is_constructed())
            return 0;

        return (mmchs->PSTATE.reg & PSTATE_CINS) >> PSTATE_CINS_SHIFT;
    }

    /**
     * @brief   Check whether the card is write protected
     *
     * @return  0  if the card is not write protected
     *          1  if the card is write protected
     *
     * @note    This is only meaningful when the controller instance has a dedicated write protect pin.
     *          Otherwise the write protect detection mechanism is application specific.
     **/
    uint32_t is_card_write_protected() noexcept
    {
        if (!is_constructed())
            return 0;

        return (mmchs->PSTATE.reg & PSTATE_WP) >> PSTATE_WP_SHIFT;
    }

    /**
     * @brief   Save the register context of the MMCSD controller
     *
     * @param   contextPtr  Structure receiving the MMCSD register context
     **/
    void context_save(MMCSDContext_t * contextPtr) noexcept
    {
        if (!is_constructed() || (contextPtr == nullptr))
            return;

        contextPtr->capa         = mmchs->CAPA.reg;
        contextPtr->systemConfig = mmchs->SYSCONFIG.reg;
        contextPtr->ctrlInfo     = mmchs->CON.reg;
        contextPtr->sysCtl       = mmchs->SYSCTL.reg;
        contextPtr->pState       = mmchs->PSTATE.reg;
        contextPtr->hctl         = mmchs->HCTL.reg;
    }

    /**
     * @brief   Restore the register context of the MMCSD controller
     *
     * @param   contextPtr  Structure holding a context previously filled by context_save()
     *
     * @note    SD_PSTATE is a read-only status register and is therefore not restored; the saved copy is
     *          kept in the context for diagnostics only.
     **/
    void context_restore(const MMCSDContext_t * contextPtr) noexcept
    {
        if (!is_constructed() || (contextPtr == nullptr))
            return;

        mmchs->SYSCONFIG.reg = contextPtr->systemConfig;
        mmchs->SYSCTL.reg    = contextPtr->sysCtl;
        mmchs->CAPA.reg      = contextPtr->capa;
        mmchs->CON.reg       = contextPtr->ctrlInfo;
        mmchs->HCTL.reg      = contextPtr->hctl;
    }

}   // namespace HAL::MMCSD
