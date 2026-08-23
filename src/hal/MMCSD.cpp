#include "hal/MMCSD.hpp"
#include "regs/MMCHS.hpp"

namespace HAL::MMCSD
{
    using namespace REGS::MMCHS;
    
    static AM335x_MMCHS_Type * mmchs = nullptr;

    
    void construct_mmcsd(void * base_addr) noexcept 
    {
        mmchs = static_cast<AM335x_MMCHS_Type *>(base_addr);
    }
    
    /**
     * @brief   Soft reset the MMC/SD controller
     *
     * @return   0   on reset success
     *          -1   on reset fail
     *
     **/
    int  soft_reset() noexcept
    {
        volatile unsigned int timeout = 0xFFFF;

        constexpr uint32_t MMCHS_SYSCONFIG_SOFTRESET = 0x00000002u;
        constexpr uint32_t MMCHS_SYSSTATUS_RESETDONE = 0x00000001u;
    
        mmchs->SYSCONFIG.reg |= MMCHS_SYSCONFIG_SOFTRESET;
    
        do 
        {
            if ((mmchs->SYSSTATUS.reg & MMCHS_SYSSTATUS_RESETDONE) == MMCHS_SYSSTATUS_RESETDONE)
                break;
        } while(timeout--);
    
        if (0 == timeout)
            return -1;
    
        return 0;
    }
    
    /**
     * @brief   Soft reset the MMC/SD controller lines
     *
     * @param   flag          reset flags indicating the lines to be reset.
     *
     * flag can take the following values
     * HS_MMCSD_DATALINE_RESET
     * HS_MMCSD_CMDLINE_RESET
     * HS_MMCSD_ALL_RESET
     *
     * @return   0   on reset success
     *          -1   on reset fail
     *
     **/
    int  lines_reset(const uint32_t flag) noexcept
    {
        volatile uint32_t timeout = 0xFFFF;
    
        mmchs->SYSCTL.reg |= flag;
    
        do
        {
            if ((mmchs->SYSCTL.reg  & flag) == flag)
                break;
        } while(timeout--);
    
        if (0 == timeout)
            return -1;

        return 0;
    }
    
    /**
     * @brief   Configure the MMC/SD controller standby, idle and wakeup modes
     *
     * @param   config        The standby, idle and wakeup modes
     *
     * flag can take the values (or a combination of the following)
     *     HS_MMCSD_STANDBY_xxx - Standby mode configuration
     *     HS_MMCSD_CLOCK_xxx - Clock mode configuration
     *     HS_MMCSD_SMARTIDLE_xxx - Smart IDLE mode configuration
     *     HS_MMCSD_WAKEUP_xxx - Wake up configuration
     *     HS_MMCSD_AUTOIDLE_xxx - Auto IDLE mode configuration
     *
     **/
    void  system_config(const uint32_t config) noexcept
    {
        constexpr uint32_t MMCHS_SYSCONFIG_STANDBYMODE     =  0x00003000u;
        constexpr uint32_t MMCHS_SYSCONFIG_CLOCKACTIVITY   =  0x00000300u;
        constexpr uint32_t MMCHS_SYSCONFIG_SIDLEMODE       =  0x00000018u;
        constexpr uint32_t MMCHS_SYSCONFIG_ENAWAKEUP       =  0x00000004u;
        constexpr uint32_t MMCHS_SYSCONFIG_AUTOIDLE        =  0x00000001u;

        mmchs->SYSCONFIG.reg  &= ~(MMCHS_SYSCONFIG_STANDBYMODE |
                                   MMCHS_SYSCONFIG_CLOCKACTIVITY |
                                   MMCHS_SYSCONFIG_SIDLEMODE |
                                   MMCHS_SYSCONFIG_ENAWAKEUP |
                                   MMCHS_SYSCONFIG_AUTOIDLE);
    
        mmchs->SYSCONFIG.reg |= config;
    }
    
    /**
     * @brief   Configure the MMC/SD bus width
     *
     * @param   width         SD/MMC bus width
     *
     * width can take the values
     *     HS_MMCSD_BUS_WIDTH_8BIT
     *     HS_MMCSD_BUS_WIDTH_4BIT
     *     HS_MMCSD_BUS_WIDTH_1BIT
     *
     **/
    void  bus_width_set(uint32_t width) noexcept
    {
        constexpr uint32_t MMCHS_CON_DW8            = 0x00000020u;
        constexpr uint32_t MMCHS_HCTL_DTW           = 0x00000002u;
        constexpr uint32_t MMCHS_HCTL_DTW_SHIFT     = 0x00000001u;
        constexpr uint32_t MMCHS_HCTL_DTW_1_BITMODE = 0x0u;
        constexpr uint32_t MMCHS_HCTL_DTW_4_BITMODE = 0x1u;
        auto wth = static_cast<e_BUS_WIDTH>(width);

        switch (wth)
        {
            case WIDTH_8BIT:
                mmchs->CON.reg |= MMCHS_CON_DW8;
                break;
    
            case WIDTH_4BIT:
                mmchs->CON.reg &= ~MMCHS_CON_DW8;
                mmchs->HCTL.reg |= (MMCHS_HCTL_DTW_4_BITMODE << MMCHS_HCTL_DTW_SHIFT);
                break;
    
            case WIDTH_1BIT:
                mmchs->CON.reg&= ~MMCHS_CON_DW8;
                mmchs->HCTL.reg &= ~MMCHS_HCTL_DTW;
                mmchs->HCTL.reg |= (MMCHS_HCTL_DTW_1_BITMODE << MMCHS_HCTL_DTW_SHIFT);
                break;
        }
    }
    
    /**
     * @brief   Configure the MMC/SD bus voltage
     *
     * @param   volt          SD/MMC bus voltage
     *
     * volt can take the values
     *     HS_MMCSD_BUS_VOLT_1P8
     *     HS_MMCSD_BUS_VOLT_3P0
     *     HS_MMCSD_BUS_VOLT_3P3
     *
     **/
    void  bus_volt_set(const uint32_t volt) noexcept
    {
        constexpr uint32_t MMCHS_HCTL_SDVS = 0x00000E00u;

        mmchs->HCTL.reg &= ~MMCHS_HCTL_SDVS;
        mmchs->HCTL.reg |= volt;
    }
    
    /**
     * @brief   Turn MMC/SD bus power on / off
     *
     * @param   pwr           power on / off setting
     *
     * pwr can take the values
     *     HS_MMCSD_BUS_POWER_ON
     *     HS_MMCSD_BUS_POWER_OFF
     *
     * @return  0 if the operation succeeded
     *         -1 if the operation failed
     *
     **/
    int  bus_power(const uint32_t pwr) noexcept
    {
        volatile uint32_t timeout = 0xFFFFF;
        constexpr uint32_t MMCHS_HCTL_SDBP = 0x00000100u;
        constexpr uint32_t MMCHS_HCTL_SDBP_SHIFT = 0x00000008u;

        constexpr uint32_t MMCHS_HCTL_SDBP_PWRON = 0x1u;
        constexpr uint32_t HS_MMCSD_BUS_POWER_ON = MMCHS_HCTL_SDBP_PWRON << MMCHS_HCTL_SDBP_SHIFT;
    
        mmchs->HCTL.reg = (mmchs->HCTL.reg & ~MMCHS_HCTL_SDBP) | pwr;
    
        if (pwr == HS_MMCSD_BUS_POWER_ON)
        {
            do
            {
                if ((mmchs->HCTL.reg& MMCHS_HCTL_SDBP) != 0)
                    break;

            } while(timeout--);
        }
    
        if (timeout == 0)
            return -1;

        return 0;
    }
    
    /**
     * @brief   Turn Internal clocks on / off
     *
     * @param   pwr           clock on / off setting
     *
     * pwr can take the values
     *     HS_MMCSD_INTCLOCK_ON
     *     HS_MMCSD_INTCLOCK_OFF
     *
     * @return  0 if the operation succeeded
     *         -1 if the operation failed
     *
     **/
    int int_clock(const uint32_t pwr) noexcept
    {
        constexpr uint32_t MMCHS_SYSCTL_ICE             = 0x00000001u;
        constexpr uint32_t MMCHS_SYSCTL_ICE_SHIFT       = 0x00000000u;
        constexpr uint32_t MMCHS_SYSCTL_ICE_OSCILLATE   = 0x1u;
        constexpr uint32_t HS_MMCSD_INTCLOCK_ON  = MMCHS_SYSCTL_ICE_OSCILLATE << MMCHS_SYSCTL_ICE_SHIFT;

        mmchs->SYSCTL.reg = (mmchs->SYSCTL.reg & ~MMCHS_SYSCTL_ICE) | pwr;
    
        if (pwr == HS_MMCSD_INTCLOCK_ON)
        {
            if(is_int_clock_stable(0xFFFF) == 0)
            {
                return -1;
            }
        }
    
        return 0;
    }
    
    /**
     * @brief   Get the internal clock stable status
     *
     * @param   retry         retry times to poll for stable
     *
     * @note : if retry is zero the status is not polled. If it is non-zero status
     *        is polled for retry times
     *
     * @return  1 if the clock is stable
     *          0 if the clock is not stable
     *
     **/
    uint32_t  is_int_clock_stable(uint32_t retry) noexcept
    {
        volatile uint32_t status = 0;
        constexpr uint32_t MMCHS_SYSCTL_ICS   = 0x00000002u;
        constexpr uint32_t MMCHS_SYSCTL_ICS_SHIFT   = 0x00000001u;
    
        do
        {
            status = (mmchs->SYSCTL.reg & MMCHS_SYSCTL_ICS) >> MMCHS_SYSCTL_ICS_SHIFT;
            if ((status == 1) || (retry == 0))
                break;

        } while (retry--);
    
        return status;
    }
    
    /**
     * @brief   Set the supported voltage list
     *
     * @param   volt          Supported bus voltage
     *
     * volt can take the values (or a combination of)
     *     HS_MMCSD_SUPPORT_VOLT_1P8
     *     HS_MMCSD_SUPPORT_VOLT_3P0
     *     HS_MMCSD_SUPPORT_VOLT_3P3
     *
     **/
    void  supported_volt_set(const uint32_t volt) noexcept
    {
        constexpr uint32_t MMCHS_CAPA_VS18 = 0x04000000u;
        constexpr uint32_t MMCHS_CAPA_VS30 = 0x02000000u;
        constexpr uint32_t MMCHS_CAPA_VS33 = 0x01000000u;

        mmchs->CAPA.reg &= ~(MMCHS_CAPA_VS18 | MMCHS_CAPA_VS30 | MMCHS_CAPA_VS33);
        mmchs->CAPA.reg |= volt;
    }
    
    /**
     * @brief   Check if the controller supports high speed
     *
     *
     * @return  0 if high speed is not supported
     *          1 if high speed is supported
     *
     **/
    uint32_t  is_hsupported() noexcept
    {
        constexpr uint32_t MMCHS_CAPA_HSS = 0x00200000u;
        constexpr uint32_t MMCHS_CAPA_HSS_SHIFT = 0x00000015u;

        return (mmchs->CAPA.reg & MMCHS_CAPA_HSS) >> MMCHS_CAPA_HSS_SHIFT;
    }
    
    /**
     * @brief   Set data timeout value
     *
     * @param   timeout       the data time out value
     *
     * Timeout value is the exponential of 2, as mentioned in the controller
     * reference manual.
     *
     * @note
: Please use HS_MMCSD_DATA_TIMEOUT(n) for setting this value
     *        13 <= n <= 27
     *
     **/
    void  data_timeout_set(const uint32_t timeout) noexcept
    {
        constexpr uint32_t MMCHS_SYSCTL_DTO = 0x000F0000u;

        mmchs->SYSCTL.reg &= ~(MMCHS_SYSCTL_DTO);
        mmchs->SYSCTL.reg |= timeout;
    }
    
    /**
     * @brief   Set output bus frequency
     *
     * @param   freq_in       The input/ref frequency to the controller
     * @param   freq_out      The required output frequency on the bus
     * @param   bypass        If the reference clock is to be bypassed
     *
     * @return   0  on clock enable success
     *          -1  on clock enable fail
     *
     * @note : If the clock is set properly, the clocks are enabled to the card with
     * the return of this function
     **/
    int  bus_freq_set(const uint32_t freq_in, const uint32_t freq_out, const uint32_t bypass) noexcept
    {
        volatile uint32_t clkd = 0;
    	volatile uint32_t regVal = 0;
        constexpr uint32_t MMCHS_SYSCTL_ICE_SHIFT       = 0x00000000u;
        constexpr uint32_t MMCHS_SYSCTL_ICE_OSCILLATE   = 0x1u;
        constexpr uint32_t HS_MMCSD_INTCLOCK_ON = MMCHS_SYSCTL_ICE_OSCILLATE << MMCHS_SYSCTL_ICE_SHIFT;
        constexpr uint32_t  MMCHS_SYSCTL_CLKD           = 0x0000FFC0u;
        constexpr uint32_t  MMCHS_SYSCTL_CLKD_SHIFT     = 0x00000006u;
        constexpr uint32_t  MMCHS_SYSCTL_CEN            = 0x00000004u;
    
        /* First enable the internal clocks */
        if (int_clock(HS_MMCSD_INTCLOCK_ON) == -1)
            return -1;

        if (bypass == 0)
        {
            /* Calculate and program the divisor */
            clkd = freq_in / freq_out;
            clkd = (clkd < 2) ? 2 : clkd;
            clkd = (clkd > 1023) ? 1023 : clkd;
    
    		/* Do not cross the required freq */
    		while((freq_in/clkd) > freq_out)
    		{
    			if (clkd == 1023)
    			{
    				/* Return - we cannot set the clock freq */
    			   return -1;
    			}
    
    			clkd++;
    		}
    
            regVal = mmchs->SYSCTL.reg & ~MMCHS_SYSCTL_CLKD;
            mmchs->SYSCTL.reg = regVal | (clkd << MMCHS_SYSCTL_CLKD_SHIFT);
    
            /* Wait for the interface clock stabilization */
            if(is_int_clock_stable(0xFFFF) == 0)
            {
                return -1;
            }
    
            /* Enable clock to the card */
            mmchs->SYSCTL.reg |= MMCHS_SYSCTL_CEN;
        }
    
        return 0;
    }
    
    /**
     * @brief   Sends INIT stream to the card
     *
     *
     * @return   0  If INIT command could be initiated
     *          -1  If INIT command could not be completed/initiated
     *
     **/
    int  init_stream_send() noexcept
    {
        constexpr uint32_t MMCHS_ISE_CC_SIGEN = 0x00000001u;
        constexpr uint32_t HS_MMCSD_SIGEN_CMDCOMP = MMCHS_ISE_CC_SIGEN;
        constexpr uint32_t MMCHS_CON_INIT = 0x00000002u;

        /* Enable the command completion status to be set */
        intr_status_enable(HS_MMCSD_SIGEN_CMDCOMP);
    
        /* Initiate the INIT command */
        mmchs->CON.reg |= MMCHS_CON_INIT;
        mmchs->CMD.reg = 0x00;
    
        int status = static_cast<int>(is_cmd_complete(0xFFFF));
    
        mmchs->CON.reg &= ~MMCHS_CON_INIT;

        /* Clear all status */
        intr_status_clear(0xFFFFFFFF);
    
        return status;
    }
    
    /**
     * @brief   Enables the controller events to set flags in status register
     *
     * @param   flag        Specific event required;
     *
     * flag can take the following (or combination of) values
     * HS_MMCSD_INTR_xxx
     *
     * @note : This function only enables the reflection of events in status register.
     * To enable events to generate a h/w interrupt request \see HSMMCSDIntrEnable()
     *
     **/
    void  intr_status_enable(const uint32_t flag) noexcept
    {
        mmchs->IE.reg |= flag;
    }
    
    /**
     * @brief   Disables the controller events to set flags in status register
     *
     * @param   flag        Specific event required;
     *
     * flag can take the following (or combination of) values
     * HS_MMCSD_INTR_xxx
     *
     * @note : This function only enables the reflection of events in status register.
     * To disable events to generate a h/w interrupt request \see HSMMCSDIntrEnable()
     *
     **/
    void  intr_status_disable(const uint32_t flag) noexcept
    {
        mmchs->IE.reg &= ~flag;
    }
    
    /**
     * @brief   Enables the controller events to generate a h/w interrupt request
     *
     * @param   flag        Specific event required;
     *
     * flag can take the following (or combination of) values
     * HS_MMCSD_SIGEN_xxx
     *
     **/
    void  intr_enable(const uint32_t flag) noexcept
    {
        mmchs->ISE.reg |= flag;
    	intr_status_enable(flag);
    }
    
    /**
     * @brief   Gets the status bits from the controller
     *
     * @param   flag        Specific status required;
     *
     *          flag can take the following (or combination of) values
     *          HS_MMCSD_STAT_xxx
     *
     * @return   status flags
     *
     **/
    uint32_t  intr_status_get(const uint32_t flag) noexcept
    {
        return mmchs->STAT.reg & flag;
    }
    
    /**
     * @brief   Clear the status bits from the controller
     *
     * @param   flag        Specific status required;
     *
     *      flag can take the following (or combination of) values
     *      HS_MMCSD_STAT_xxx
     *      
     **/
    void  intr_status_clear(const uint32_t flag) noexcept
    {
        mmchs->STAT.reg = flag;
    }
    
    /**
     * @brief    Checks if the command is complete
     *
     * @param    retry       retry times to poll for completion
     *
     * @return   1 if the command is complete
     *           0 if the command is not complete
     **/
    uint32_t  is_cmd_complete(uint32_t retry) noexcept
    {
        volatile uint32_t status = 0;
        constexpr uint32_t MMCHS_STAT_CC         = 0x00000001u;
        constexpr uint32_t MMCHS_STAT_CC_SHIFT   = 0x00000000u;
    
        do 
        {
            status = (mmchs->STAT.reg & MMCHS_STAT_CC) >> MMCHS_STAT_CC_SHIFT;

            if (( 1 == status) || (0  == retry))
                break;

        } while (retry--);
    
        return status;
    }
    
    /**
     * @brief    Checks if the transfer is complete
     *
     * @param    retry       retry times to poll for completion
     *
     * @return   1 if the transfer is complete
     *           0 if the transfer is not complete
     **/
    uint32_t  is_xfer_complete(uint32_t retry) noexcept
    {
        volatile uint32_t status = 0;
        constexpr uint32_t MMCHS_STAT_TC       = 0x00000002u;
        constexpr uint32_t MMCHS_STAT_TC_SHIFT = 0x00000001u;

        do 
        {
            status = (mmchs->STAT.reg & MMCHS_STAT_TC) >> MMCHS_STAT_TC_SHIFT;

            if ((1 == status) || (0 == retry))
                break;

        } while (retry--);
    
        return status;
    }
    
    /**
     * @brief    Set the block length/size for data transfer
     *
     * @param    blklen      Command to be passed to the controller/card
     *
     * @note: blklen should be within the limits specified by the controller/card
     *
     **/
    void  blk_len_set(uint32_t blklen) noexcept
    {
        constexpr uint32_t MMCHS_BLK_BLEN = 0x00000FFFu;

        mmchs->BLK.reg &= ~MMCHS_BLK_BLEN;
        mmchs->BLK.reg |= blklen;
    }
    
    /**
     * @brief    Pass the MMC/SD command to the controller/card
     *
     * @param   cmd         Command to be passed to the controller/card
     * @param   cmdarg      argument for the command
     * @param   data        data pointer, if it is a data command, else must be null
     * @param   nblks       data length in number of blocks (multiple of BLEN)
     * @param   dmaEn       Should dma be enabled (1) or disabled (0)
     *
     * @note  : Please use HS_MMCSD_CMD(cmd, type, restype, rw) to form command
     *
     **/
    void  command_send(uint32_t cmd, uint32_t cmdarg, const void *data, uint32_t nblks, uint32_t dmaEn) noexcept
    {
        constexpr uint32_t MMCHS_CMD_DP   = 0x00200000u;
        constexpr uint32_t MMCHS_CMD_MSBS  = 0x00000020u;
        constexpr uint32_t MMCHS_CMD_BCE   = 0x00000002u;
        constexpr uint32_t MMCHS_CMD_DE   = 0x00000001u;
        constexpr uint32_t MMCHS_BLK_NBLK   = 0xFFFF0000u;
        constexpr uint32_t MMCHS_BLK_NBLK_SHIFT   = 0x00000010u;

        if (nullptr != data)
            cmd |= (MMCHS_CMD_DP | MMCHS_CMD_MSBS | MMCHS_CMD_BCE);

        if (1 == dmaEn)
            cmd |= MMCHS_CMD_DE;
    
        /* Set the block information; block length is specified separately */
        mmchs->BLK.reg &= ~MMCHS_BLK_NBLK;
        mmchs->BLK.reg |= nblks << MMCHS_BLK_NBLK_SHIFT;
    
        /* Set the command/command argument */
        mmchs->ARG.reg = cmdarg;
        mmchs->CMD.reg = cmd;
    
    }
    
    /**
     * @brief    Get the command response from the conntroller
     *
     * @param    rsp         pointer to buffer which is to be filled with the response
     *
     * @note    : that this function shall return the values from all response registers.
     *            Hence, rsp, must be a pointer to memory which can hold max response length.
     *            It is the responsibility of the caller to use only the required/relevant
     *            parts of the response
     *
     **/
    void  response_get(uint32_t *rsp) noexcept
    {
        for (uint32_t i = 0; i <=3; i++)
        {
            rsp[i] = mmchs->RSP(i);
        }
    }
    
    /**
     * @brief    Send the data to the card from the conntroller
     *
     * @param    data        pointer to buffer which is to be filled with the data
     * @param    len         length of the data
     *
     * @note    : this function reads the data in chunks of 32 bits (4-byte words).
     *           Hence, the len should be multiple of 4-byte words
     *
     **/
    void  data_get(unsigned char *data, const uint32_t len) noexcept
    {
        for (uint32_t i = 0; i < len/4; i++)
    	{
    		reinterpret_cast<uint32_t *>(data)[i] = mmchs->DATA.reg;
    	}
    }
    
    /**
     * @brief    Check if the card is inserted and detected
     *
     *
     * @return   0  if the card is not inserted and detected
     *           1  if the card is inserted and detected
     *
     * @note    : that this functional may not be available for all instances of the
     *          controler. This function, is only useful of the controller has a dedicated
     *          card detect pin. If not, the card detection mechanism is application
     *          implementation specific
     **/
    uint32_t  is_card_inserted() noexcept
    {
        constexpr uint32_t MMCHS_PSTATE_CINS = 0x00010000u;
        constexpr uint32_t MMCHS_PSTATE_CINS_SHIFT = 0x00000010u;

        return (mmchs->PSTATE.reg & MMCHS_PSTATE_CINS) >> MMCHS_PSTATE_CINS_SHIFT;
    }
    
    /**
     * @brief    Check if the card is write protected
     *
     *
     * @return   0  if the card is not write protected
     *           1  if the card is write protected
     * @note   : that this functional may not be available for all instances of the
     *          controler. This function, is only useful of the controller has a dedicated
     *          write protect detect pin. If not, the write protect detection mechanism is
     *          application implementation specific
     **/
    uint32_t  is_card_write_protected() noexcept
    {
        constexpr uint32_t MMCHS_PSTATE_WP = 0x00080000u;
        constexpr uint32_t MMCHS_PSTATE_WP_SHIFT = 0x00000013u;

        return (mmchs->PSTATE.reg & MMCHS_PSTATE_WP) >> MMCHS_PSTATE_WP_SHIFT;
    }
    
    /**
     * @brief   This API can be used to save the register context of MMCSD
     *
     * @param   contextPtr    Pointer to the structure where MMCSD register
     *                        context need to be saved
     *
     *
     **/
    void  context_save(MMCSDContext_t *contextPtr) noexcept
    {
        contextPtr->capa = mmchs->CAPA.reg;
        contextPtr->systemConfig = mmchs->SYSCONFIG.reg;
        contextPtr->ctrlInfo = mmchs->CON.reg;
        contextPtr->sysCtl = mmchs->SYSCTL.reg;
        contextPtr->pState = mmchs->PSTATE.reg;
        contextPtr->hctl = mmchs->HCTL.reg;
    }    
    
    /**
     * @brief   This API can be used to restore the register context of MMCSD
     *
     * @param   contextPtr    Pointer to the structure where MMCSD register
     *                        context need to be saved
     *
     **/
    void  context_restore(const MMCSDContext_t *contextPtr) noexcept
    {
        mmchs->SYSCONFIG.reg = contextPtr->systemConfig;
        mmchs->SYSCTL.reg = contextPtr->sysCtl;
        mmchs->CAPA.reg = contextPtr->capa;
        mmchs->CON.reg = contextPtr->ctrlInfo;
        mmchs->HCTL.reg = contextPtr->hctl;
        mmchs->PSTATE.reg = contextPtr->pState;
    }
}