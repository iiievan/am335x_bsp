#include "../../include/hal/EDMA/EDMA.hpp"
#include "hal/EDMA/InterruptDispatcher.hpp"
#include "hal/INTC.hpp"

namespace HAL::EDMA
{
    using namespace REGS::EDMA;
    static e_REGION_ID  region_id = e_REGION_ID::REGION_0;

    e_REGION_ID get_region_id() noexcept { return region_id; }
    void set_region_id(const e_REGION_ID regid) noexcept
    {
        region_id = (regid >= REGIONS_MAX) ? static_cast<e_REGION_ID>(REGIONS_MAX - 1U) : regid;
    }

    inline void check_and_set_bit(volatile uint32_t& reg, uint32_t bit_index) noexcept
    {
        const uint32_t mask = (1u << bit_index);
        if ((reg & mask) == 0u) { reg |= mask; }
    }

    void  module_clock_config() noexcept
    {
        using namespace REGS::PRCM;
        auto& per = *AM335x_CM_PER;

        per.TPCC_CLKCTRL.b.MODULEMODE = MODULEMODE_ENABLE;
        while(per.TPCC_CLKCTRL.b.MODULEMODE != MODULEMODE_ENABLE) {}

        per.TPTC0_CLKCTRL.b.MODULEMODE = MODULEMODE_ENABLE;
        while(per.TPTC0_CLKCTRL.b.MODULEMODE != MODULEMODE_ENABLE) {}

        per.TPTC1_CLKCTRL.b.MODULEMODE = MODULEMODE_ENABLE;
        while(per.TPTC1_CLKCTRL.b.MODULEMODE != MODULEMODE_ENABLE) {}

        per.TPTC2_CLKCTRL.b.MODULEMODE = MODULEMODE_ENABLE;
        while(per.TPTC2_CLKCTRL.b.MODULEMODE != MODULEMODE_ENABLE) {}

        /*	DMA in non-idle mode */
        AM335X_EDMA3TC0->SYSCONFIG.reg = 0x00000028;
        AM335X_EDMA3TC1->SYSCONFIG.reg = 0x00000028;
        AM335X_EDMA3TC2->SYSCONFIG.reg = 0x00000028;

        while(per.TPCC_CLKCTRL.b.IDLEST != IDLEST_FUNC) {}

        while(per.TPTC0_CLKCTRL.b.IDLEST != IDLEST_FUNC) {}
        while(per.TPTC0_CLKCTRL.b.STBYST != STBYST_FUNC) {}

        while(per.TPTC1_CLKCTRL.b.IDLEST != IDLEST_FUNC) {}
        while(per.TPTC1_CLKCTRL.b.STBYST != STBYST_FUNC) {}

        while(per.TPTC2_CLKCTRL.b.IDLEST != IDLEST_FUNC) {}
        while(per.TPTC2_CLKCTRL.b.STBYST != STBYST_FUNC) {}
    }

    /**
     *  @brief   EDMA3 Initialization
     *
     *  This function initializes the EDMA3 Driver
     *  Clears the error specific registers (EMCR/EMCRh, QEMCR, CCERRCLR) &
     *  initialize the Queue Number Registers
     *
     *  @param   que_num                   Event Queue Number to which the channel
     *                                   will be mapped (valid only for the
     *                                   Master Channel (DMA/QDMA) request).
     *
     *  @return None
     *
     *  Note :   The region_id is the shadow region(0 or 1) used and the,
     *          Event Queue used is either (0 or 1). There are only four shadow
     *          regions and only two event Queues
     */
    void init(const e_EVENT_QUEUE que_num) noexcept
    {
        using namespace HAL::INTC;
        auto& cc = *AM335X_EDMA3CC;
        uint32_t count = 0;

        // Clear the Event miss Registers
        cc.EMCR.reg     =  SET_ALL_BITS;
        cc.EMCRH.reg    =  SET_ALL_BITS;
        cc.QEMCR.reg    =  SET_ALL_BITS;
        cc.CCERRCLR.reg =  SET_ALL_BITS;

        // FOR TYPE EDMA
        // Enable the DMA (0 - 64) channels in the DRAE and DRAEH register
        cc.DRAE(region_id).reg =  SET_ALL_BITS;
        cc.DRAEH(region_id).reg =  SET_ALL_BITS;

        if(EDMA_REVID == version_get())
        {
            for(uint32_t i = 0; i <  AM335X_DMACH_MAX; i++)
            {
                cc.DCHMAP[i].reg = i << 5;
            }
        }

        // Initialize the DMA Queue Number Registers
        //for (uint32_t count = 0; count < SOC_EDMA3_NUM_DMACH; count++)
        for (count = 0; count <  AM335X_DMACH_MAX; count++)
        {
            cc.DMAQNUM[count >> 3u].reg &=  DMAQNUM_CLR(count);
            cc.DMAQNUM[count >> 3u].reg |=  DMAQNUM_SET(count, que_num);
        }

        // FOR TYPE QDMA
        // Enable the DMA (0 - 64) channels in the DRAE register
        cc.QRAE[region_id].reg =  SET_ALL_BITS;

        // Initialize the QDMA Queue Number Registers
        for (count = 0; count <  AM335X_QDMACH_MAX; count++)
        {
             cc.QDMAQNUM.reg &=  QDMAQNUM_CLR(count);
            cc.QDMAQNUM.reg |=  QDMAQNUM_SET(count, que_num);
        }

        register_handler(REGS::INTC::EDMACOMPINT, reinterpret_cast<isr_handler_t>(EDMA_Completion_ISR));
        priority_set(REGS::INTC::EDMACOMPINT, 0, REGS::INTC::HOSTINT_ROUTE_IRQ);
        unmask_interrupt(REGS::INTC::EDMACOMPINT);

        register_handler(REGS::INTC::EDMAERRINT, reinterpret_cast<isr_handler_t>(EDMA_Error_ISR));
        priority_set(REGS::INTC::EDMAERRINT, 0, REGS::INTC::HOSTINT_ROUTE_IRQ);
        unmask_interrupt(REGS::INTC::EDMAERRINT);
    }

    void  set_non_idle_mode() noexcept
    {
        auto& tc0 = *AM335X_EDMA3TC0;
        auto& tc1 = *AM335X_EDMA3TC1;
        auto& tc2 = *AM335X_EDMA3TC2;

        //m_EDMA3TC0_regs.SYSCONFIG.reg = 0x00000028;
        //m_EDMA3TC1_regs.SYSCONFIG.reg = 0x00000028;
        //m_EDMA3TC2_regs.SYSCONFIG.reg = 0x00000028;

        tc0.SYSCONFIG.b.IDLEMODE    =  SYSC_SMARTIDLE;
        tc0.SYSCONFIG.b.STANDBYMODE =  SYSC_SMARTSTBY;
        tc1.SYSCONFIG.b.IDLEMODE    =  SYSC_SMARTIDLE;
        tc1.SYSCONFIG.b.STANDBYMODE =  SYSC_SMARTSTBY;
        tc2.SYSCONFIG.b.IDLEMODE    =  SYSC_SMARTIDLE;
        tc2.SYSCONFIG.b.STANDBYMODE =  SYSC_SMARTSTBY;
    }

    /**
     * @brief  Enable channel to Shadow region mapping
     *
     * This API allocates DMA/QDMA channels or TCCs, and the same resources are
     * enabled in the shadow region specific register (DRAE/DRAEH/QRAE).
     * Here only one shadow region is used since, there is only one Master.
     *
     *  @param   ch_type      (DMA/QDMA) Channel
     *                        For Example: For DMA it is,
     *                        EDMA3_CHANNEL_TYPE_DMA.
     *
     *  @param   ch_num       Allocated channel number.
     *
     *  chtype can have values
     *        EDMA3_CHANNEL_TYPE_DMA
     *        EDMA3_CHANNEL_TYPE_QDMA
     *
     */
    void enable_ch_in_shadow_reg(const e_EDMA3_CH_TYPE ch_type,const uint32_t ch_num) noexcept
    {
        auto& cc = *AM335X_EDMA3CC;

        // Allocate the DMA/QDMA channel
        if (CHANNEL_TYPE_DMA == ch_type || CHANNEL_TYPE_QDMA == ch_type)
        {
             if(ch_num < 32)
                 check_and_set_bit(cc.DRAE(region_id).reg, ch_num);
             else
                 check_and_set_bit(cc.DRAEH(region_id).reg, ch_num - 32u);
        }

        if(CHANNEL_TYPE_QDMA == ch_type)
        {
            check_and_set_bit(cc.QRAE[region_id].reg, ch_num);
        }
    }

    /**
     * @brief  Disable channel to Shadow region mapping
     *
     * This API allocates DMA/QDMA channels or TCCs, and the same resources are
     * enabled in the shadow region specific register (DRAE/DRAEH/QRAE).
     * Here only one shadow region is used since, there is only one Master.
     *
     * @param   ch_type    (DMA/QDMA) Channel
     *                      For Example: For DMA it is,
     *                      EDMA3_CHANNEL_TYPE_DMA.
     *
     * @param   ch_num      Allocated channel number.
     *
     *  ch_type can have values
     *        EDMA3_CHANNEL_TYPE_DMA
     *        EDMA3_CHANNEL_TYPE_QDMA
     *
     */
    void disable_ch_in_shadow_reg(const e_EDMA3_CH_TYPE ch_type, const uint32_t ch_num) noexcept
    {
        auto& cc = *AM335X_EDMA3CC;

        /* Allocate the DMA/QDMA channel */
        if (CHANNEL_TYPE_DMA == ch_type || CHANNEL_TYPE_QDMA == ch_type)
        {
             if(ch_num < 32)
                  cc.DRAE(region_id).reg &= ~(0x01u << ch_num);
             else
                  cc.DRAEH(region_id).reg &= ~(0x01u << (ch_num - 32));
        }

        if (CHANNEL_TYPE_QDMA == ch_type)
        {
            cc.QRAE[region_id].reg &= ~(0x01u << ch_num);
        }
    }

    /**
     *  @brief  Map channel to Event Queue
     *
     *  This API maps DMA/QDMA channels to the Event Queue
     *
     *  @param  ch_type     (DMA/QDMA) Channel
     *                     For Example: For QDMA it is
     *                     EDMA3_CHANNEL_TYPE_QDMA.
     *
     *  @param  ch_num      Allocated channel number.
     *
     *  @param  evt_Qnum    Event Queue Number to which the channel
     *                     will be mapped (valid only for the
     *                     Master Channel (DMA/QDMA) request).
     *
     *  ch_type can have values
     *        EDMA3_CHANNEL_TYPE_DMA
     *        EDMA3_CHANNEL_TYPE_QDMA
     *
     */
    void map_ch_to_evtQ(const e_EDMA3_CH_TYPE ch_type, const uint32_t ch_num,  const e_EVENT_QUEUE evt_Qnum) noexcept
    {
        auto& cc = *AM335X_EDMA3CC;

        if (CHANNEL_TYPE_DMA == ch_type)
        {
            cc.DMAQNUM[ch_num >> 3u].reg &=  DMAQNUM_CLR(ch_num);
            cc.DMAQNUM[ch_num >> 3u].reg |=  DMAQNUM_SET(ch_num, evt_Qnum);
        }
        else
        if (CHANNEL_TYPE_QDMA == ch_type)
        {
            cc.QDMAQNUM.reg &=  QDMAQNUM_CLR(ch_num);
            cc.QDMAQNUM.reg |=  QDMAQNUM_SET(ch_num, evt_Qnum);
        }
    }

    /**
     *  @brief  Remove Mapping of channel to Event Queue
     *
     *  This API Unmaps DMA/QDMA channels to the Event Queue allocated
     *
     *  @param  ch_type     (DMA/QDMA) Channel
     *                     For Example: For DMA it is
     *                     EDMA3_CHANNEL_TYPE_DMA.
     *
     *  @param  ch_num      Allocated channel number.
     *
     *  ch_type can have values
     *        EDMA3_CHANNEL_TYPE_DMA
     *        EDMA3_CHANNEL_TYPE_QDMA
     *
     */
    void unmap_ch_to_evtQ(const e_EDMA3_CH_TYPE ch_type, const uint32_t ch_num) noexcept
    {
        auto& cc = *AM335X_EDMA3CC;

        if (CHANNEL_TYPE_DMA == ch_type)
            cc.DMAQNUM[ch_num >> 3u].reg &=  DMAQNUM_CLR(ch_num);
        else
        if (CHANNEL_TYPE_QDMA == ch_type)
            cc.QDMAQNUM.reg &=  QDMAQNUM_CLR(ch_num);
    }

    /**
     *  @brief  Enables the user to map a QDMA channel to PaRAM set
     *          This API Needs to be called before programming the paRAM sets for
     *          the QDMA Channels.Application needs to maitain the paRAMId
     *          provided by this API.This paRAMId is used to set paRAM and get
     *          paRAM. Refer corresponding API's for more details.
     *
     *  @param  ch_num                    Allocated QDMA channel number.
     *
     *  @param  paRAM_id                  PaRAM Id to which the QDMA channel will be
     *                                   mapped to.
     *
     *  @return None
     *
     *  Note : The PaRAMId requested must be greater than 32(SOC_EDMA3_NUM_DMACH).
     *         and lesser than SOC_EDMA3_NUM_DMACH + ch_num  Because, the first
     *         32 PaRAM's are directly mapped to first 32 DMA channels and (32 - 38)
     *         for QDMA Channels. (32 - 38) is assigned by driver in this API.
     *
     */
    bool map_QDMA_ch_to_paRAM(const uint32_t ch_num, uint32_t& paRAM_id) noexcept
    {
        using namespace REGS::EDMA;
        auto& cc = *AM335X_EDMA3CC;
        constexpr uint32_t QCHMAP_PAENTRY_MSK   = 0x00003FE0u; // Биты 5..13
        constexpr uint32_t QCHMAP_PAENTRY_SHIFT = 5u;

        if (ch_num >= AM335X_QDMACH_MAX)
        {
            return false;
        }

        // Если передан некорректный paRAM_id, назначаем безопасный дефолт (32..39)
        if (paRAM_id >= AM335x_PARAMSETS_MAX)
        {
            paRAM_id = AM335x_QDMA_PARAM_BASE + ch_num;
        }

        uint32_t reg_val = cc.QCHMAP[ch_num].reg;
        reg_val &= ~QCHMAP_PAENTRY_MSK;
        reg_val |= ((paRAM_id << QCHMAP_PAENTRY_SHIFT) & QCHMAP_PAENTRY_MSK);

        cc.QCHMAP[ch_num].reg = reg_val;

        return true;
    }

    /**
     * @brief  Assign a Trigger Word to the specified QDMA channel
     *
     * This API sets the Trigger word for the specific QDMA channel in the QCHMAP
     * Register. Default QDMA trigger word is CCNT.
     *
     * @param  ch_num               QDMA Channel which needs to be assigned
     *                             the Trigger Word
     *
     * @param  trig_word            The Trigger Word for the QDMA channel.
     *                             Trigger Word is the word in the PaRAM
     *                             Register Set which, when written to by CPU,
     *                             will start the QDMA transfer automatically.
     *
     */
    void set_QDMA_trig_word(const uint32_t ch_num, const uint8_t trig_word) noexcept
    {
        constexpr uint32_t MSK   = 0x0000001Cu;
        constexpr uint32_t SHIFT = 2u;

        auto& reg = AM335X_EDMA3CC->QCHMAP[ch_num].reg;
        reg = (reg & ~MSK) | ((static_cast<uint32_t>(trig_word) << SHIFT) & MSK);
    }

    /**
     *  @brief   Enables the user to Clear any missed event
     *
     *  @param   ch_num                  Allocated channel number.
     *
     */
    void clr_miss_evt(const uint32_t ch_num) noexcept
    {
        auto& cc = *AM335X_EDMA3CC;

        if(ch_num < 32)
        {
             cc.S_SECR(region_id).reg = (0x01u << ch_num);    // clear SECR to clean any previous NULL request
             cc.EMCR.reg |= (0x01u <<  ch_num);               // clear EMCR to clean any previous NULL request
        }
        else
        {
             cc.S_SECRH(region_id).reg = (0x01u << (ch_num - 32));
             cc.EMCRH.reg |= (0x01u <<  (ch_num - 32));               // clear EMCRH to clean any previous NULL request
        }
    }

    /**
     *  @brief   Enables the user to Clear any QDMA missed event
     *
     *  @param   ch_num                  Allocated channel number.
     *
     */
    void QDMA_clr_miss_evt(const uint32_t ch_num) noexcept
    {
        auto& cc = *AM335X_EDMA3CC;

        // clear SECR and EMCR to clean any previous NULL request
        cc.S_QSECR(region_id).reg = (0x01u << ch_num);
        cc.QEMCR.reg |= (0x01u <<  ch_num);
    }

    /**
     *  @brief   Enables the user to Clear any Channel controller Errors
     *
     *  @param   flags                Masks to be passed.
     *
     *  Flags can have values:
     *
     *  EDMA3CC_CLR_TCCERR            Clears the TCCERR bit in the EDMA3CC ERR Reg
     *  EDMA3CC_CLR_QTHRQ0            Queue threshold error clear for queue 0.
     *  EDMA3CC_CLR_QTHRQ1            Queue threshold error clear for queue 1.
     */
    void clr_CC_Err(const uint32_t flags) noexcept
    {
        auto& cc = *AM335X_EDMA3CC;

        // (CCERRCLR) - clear channel controller error register
        cc.CCERRCLR.reg = flags;
    }

    /**
     *  @brief   Enables the user to Set an event. This API helps user to manually
     *           set events to initiate DMA transfer requests.
     *
     *  @param   ch_num                  Allocated channel number.
     *
     *  Note :   This API is generally used during Manual transfers.
     */
    void set_event(const uint32_t ch_num) noexcept
    {
        const auto& cc = *AM335X_EDMA3CC;
        if(ch_num < 32)
              cc.S_ESR(region_id).reg |= (0x01u <<  ch_num);
        else
              cc.S_ESRH(region_id).reg |= (0x01u << (ch_num - 32));
    }

    /**
     *  @brief   Enables the user to Clear an event.
     *
     *  @param   ch_num                  Allocated channel number.
     *
     *  Note :   This API is generally used during Manual transfers.
     */
    void clr_event(const uint32_t ch_num) noexcept
    {
        const auto& cc = *AM335X_EDMA3CC;
        if(ch_num < 32)
              cc.S_ECR(region_id).reg |= (0x01u <<  ch_num);
        else
              cc.S_ECRH(region_id).reg |= (0x01u << (ch_num - 32));
    }

    /**
     *  @brief   Enables the user to enable an DMA event.
     *
     *  @param   ch_num                  Allocated channel number.
     *
     *  Note :   Writes of 1 to the bits in EESR sets the corresponding event
     *           bits in EER. This is generally used for Event Based transfers.
     */
    void enable_DMA_event(const uint32_t ch_num) noexcept
    {
        const auto& cc = *AM335X_EDMA3CC;
        if(ch_num < 32)
             cc.S_EESR(region_id).reg |= (0x01u <<  ch_num);
        else
             cc.S_EESRH(region_id).reg |= (0x01u << (ch_num - 32));
    }

    /**
     *  @brief   Enables the user to Disable an DMA event.
     *
     *  @param   ch_num                  Allocated channel number.
     *
     *  Note :   Writes of 1 to the bits in EECR clear the corresponding event bits
     *           in EER; writes of 0 have no effect.. This is generally used for
     *           Event Based transfers.
     */
    void disable_DMA_event(const uint32_t ch_num) noexcept
    {
        const auto& cc = *AM335X_EDMA3CC;
        if(ch_num < 32)
             cc.S_EECR(region_id).reg |= (0x01u <<  ch_num);
        else
             cc.S_EECRH(region_id).reg |= (0x01u <<  ch_num);
    }

    /**
     *  @brief   Enables the user to enable an QDMA event.
     *
     *  @param   ch_num                  Allocated channel number.
     *
     *  Note :   Writes of 1 to the bits in QEESR sets the corresponding event
                  bits in QEER.
     */
    void enable_QDMA_event(const uint32_t ch_num) noexcept
    {
         AM335X_EDMA3CC->S_QEESR(region_id).reg = (0x01u << ch_num);
    }

    /**
     *  @brief   Enables the user to disable an QDMA event.
     *
     *  @param   ch_num                  Allocated channel number.
     *
     *  Note :   Writes of 1 to the bits in QEECR clears the corresponding event
                  bits in QEER.
     */
    void disable_QDMA_event(const uint32_t ch_num) noexcept
    {
         AM335X_EDMA3CC->S_QEECR(region_id).reg = (0x01u << ch_num);
    }

    /**
     *  @brief   This function returns interrupts status of those events
     *           which is less than 32.
     *
     *  @return  value                  Status of the Interrupt Pending Register
     *
     */
    uint32_t get_intr_status() noexcept
    {
        return AM335X_EDMA3CC->S_IPR(region_id).reg;
    }

    /**
     *  @brief   Enables the user to enable the transfer completion interrupt
     *           generation by the EDMA3CC for all DMA/QDMA channels.
     *
     *  @param   ch_num                  Allocated channel number.
     *
     *  Note :   To set any interrupt bit in IER, a 1 must be written to the
     *           corresponding interrupt bit in the interrupt enable set register.
     */
    void enable_evt_intr(const uint32_t ch_num) noexcept
    {
        const auto& cc = *AM335X_EDMA3CC;
        if(ch_num < 32)
            cc.S_IESR(region_id).reg |= (0x01u <<  ch_num);
        else
            cc.S_IESRH(region_id).reg |= (0x01u << (ch_num - 32));
    }

    /**
     *  @brief   Enables the user to clear CC interrupts
     *
     *  @param   ch_num                  Allocated channel number.
     *
     *  Note :   Writes of 1 to the bits in IECR clear the corresponding interrupt
     *           bits in the interrupt enable registers (IER); writes of 0 have
     *           no effect.
     */
    void disable_evt_intr(const uint32_t ch_num) noexcept
    {
        const auto& cc = *AM335X_EDMA3CC;
        if(ch_num < 32)
             cc.S_IECR(region_id).reg |= (0x01u <<  ch_num);
        else
             cc.S_IECRH(region_id).reg |= (0x01u << (ch_num - 32));
    }

    /**
     *  @brief   Enables the user to Clear an Interrupt.
     *
     *  @param   value                  Value to be set to clear the Interrupt Status.
     *
     */
    void clr_intr(const uint32_t value) noexcept
    {
        const auto& cc = *AM335X_EDMA3CC;

        if(value < 32)
            cc.S_ICR(region_id).reg = (1u << value);
        else
            cc.S_ICRH(region_id).reg = (1u << (value - 32));
    }

    /**
     *  @brief   Retrieve existing PaRAM set associated with specified logical
     *           channel (DMA/Link).
     *
     *  @param   paRAM_id   User gets the existing PaRAM here.
     *  @param        dst   distenation struct
     */
    void get_paRAM(const uint32_t paRAM_id, paRAM_entry_t& dst) noexcept
    {
        // Копируем из аппаратного регистра в структуру в памяти
        dst = AM335X_EDMA3CC->paRAM(paRAM_id);
    }
    [[nodiscard]] paRAM_entry_t get_paRAM(const uint32_t paRAM_id) noexcept
    {
        return AM335X_EDMA3CC->paRAM(paRAM_id);
    }

    /**
     * @brief   Retrieve existing PaRAM set associated with specified logical
     *          channel (QDMA).
     *
     *  @param   paRAM_id   User gets the existing PaRAM here.
     *  @param        dst   distenation struct
     */
    void QDMA_get_paRAM(const uint32_t paRAM_id, paRAM_entry_t& dst) noexcept
    {
        dst = AM335X_EDMA3CC->paRAM(paRAM_id);
    }
    [[nodiscard]] paRAM_entry_t QDMA_get_paRAM(const uint32_t paRAM_id) noexcept
    {
        return AM335X_EDMA3CC->paRAM(paRAM_id);
    }

    /**
     * @brief   Copy the user specified PaRAM Set onto the PaRAM Set associated
     *          with the logical channel (DMA/Link).
     *
     * This API takes a PaRAM Set as input and copies it onto the actual PaRAM Set
     * associated with the logical channel. OPT field of the PaRAM Set is written
     * first and the CCNT field is written last.
     *
     *
     * @param   ch_num          Logical Channel whose PaRAM set is requested.
     * @param   src             Parameter RAM set to be copied onto existing PaRAM.
     */
    void set_paRAM(const uint32_t ch_num, const paRAM_entry_t& src) noexcept
    {
        AM335X_EDMA3CC->paRAM(ch_num) = src;
    }
    void set_paRAM(const uint32_t ch_num, const paRAM_entry_t* src) noexcept
    {
        if (src) AM335X_EDMA3CC->paRAM(ch_num) = *src;
    }
    /**
     * @brief   Copy the user specified PaRAM Set onto the PaRAM Set associated
     *          with the logical channel (QDMA only).
     *
     * This API takes a PaRAM Set as input and copies it onto the actual PaRAM Set
     * associated with the logical channel. OPT field of the PaRAM Set is written
     * first and the CCNT field is written last.
     *
     *
     * @param   ch_num                 Logical Channel whose PaRAM set is
     *                                 requested.
     *
     * @param   src               Parameter RAM set to be copied onto existing
     *                                 PaRAM.
     */
    // Перегрузка с передачей указателя и выбором trigger_word (по умолчанию 7 - CCNT)
    void QDMA_set_paRAM(const uint32_t paRAM_id, const paRAM_entry_t* new_paRAM) noexcept
    {
        if (!new_paRAM) return;

        auto* dst = reinterpret_cast<volatile uint32_t*>(&AM335X_EDMA3CC->paRAM(paRAM_id));
        const auto* src = reinterpret_cast<const uint32_t*>(new_paRAM);

        for (uint32_t i = 0; i < 8; ++i)
        {
            dst[i] = src[i];
        }

        __asm__ volatile("dmb" ::: "memory");
    }

    void QDMA_set_paRAM(const uint32_t paRAM_id, const paRAM_entry_t& src) noexcept
    {
        QDMA_set_paRAM(paRAM_id, &src);
    }

    /**
     * @brief   Set a particular PaRAM set entry of the specified PaRAM set
     *
     * @param   paRAM_id           PaRAM Id to which the QDMA channel is
     *                            mapped to.
     *
     * @param   paRAM_entry        Specify the PaRAM set entry which needs
     *                            to be set.
     *
     * @param   new_paRAM_entry_val  The new field setting. Make sure this field is
     *                            packed for setting certain fields in paRAM.
     *
     *  paRAM_entry can have values:
     *
     *  EDMA3CC_PARAM_ENTRY_OPT
     *  EDMA3CC_PARAM_ENTRY_SRC
     *  EDMA3CC_PARAM_ENTRY_ACNT_BCNT
     *  EDMA3CC_PARAM_ENTRY_DST
     *  EDMA3CC_PARAM_ENTRY_SRC_DST_BIDX
     *  EDMA3CC_PARAM_ENTRY_LINK_BCNTRLD
     *  EDMA3CC_PARAM_ENTRY_SRC_DST_CIDX
     *  EDMA3CC_PARAM_ENTRY_CCNT
     *
     * Note :    This API should be used while setting the PaRAM set entry
     *          for QDMA channels. If EDMA3QdmaSetPaRAMEntry() used,
     *          it will trigger the QDMA channel before complete
     *          PaRAM set entry is written.
     */
    void QDMA_set_paRAM_entry(const uint32_t paRAM_id, const uint32_t paRAM_entry, const uint32_t new_paRAM_entry_val) noexcept
    {
        if (paRAM_entry <= static_cast<uint32_t> (e_paRAM_entry_field::CCNT))  // 0..7
        {
            auto* dist = reinterpret_cast<uint32_t*>(&AM335X_EDMA3CC->paRAM(paRAM_id));
            dist[paRAM_entry] = new_paRAM_entry_val;
        }
    }

    /**
     * @brief   Get a particular PaRAM entry of the specified PaRAM set
     *
     * @param   paRAM_id           PaRAM Id to which the QDMA channel is
     *                            mapped to.
     *
     * @param   paRAM_entry        Specify the PaRAM set entry which needs
     *                            to be read.
     *
     *  paRAM_entry can have values:
     *
     *  EDMA3CC_PARAM_ENTRY_OPT
     *  EDMA3CC_PARAM_ENTRY_SRC
     *  EDMA3CC_PARAM_ENTRY_ACNT_BCNT
     *  EDMA3CC_PARAM_ENTRY_DST
     *  EDMA3CC_PARAM_ENTRY_SRC_DST_BIDX
     *  EDMA3CC_PARAM_ENTRY_LINK_BCNTRLD
     *  EDMA3CC_PARAM_ENTRY_SRC_DST_CIDX
     *  EDMA3CC_PARAM_ENTRY_CCNT
     *
     * @return  paRAMEntryVal     The value of the paRAM field pointed by the
     *                            paRAM_entry.
     *
     * Note :    This API should be used while reading the PaRAM set entry
     *          for QDMA channels. And the paRAMEntryVal is a packed value for
     *          certain fields of paRAM_entry.The user has to make sure the value
     *          is unpacked appropriately.
     *          For example, the third field is A_B_CNT. Hence he will have to
     *          unpack it to two 16 bit fields to get ACNT and BCNT.
     */
    uint32_t QDMA_get_paRAM_entry(const uint32_t paRAM_id, const uint32_t paRAM_entry) noexcept
    {
        if (paRAM_entry > static_cast<uint32_t> (e_paRAM_entry_field::CCNT)) return 0;

        const auto* src = reinterpret_cast<uint32_t*>(&AM335X_EDMA3CC->paRAM(paRAM_id));
        return src[paRAM_entry];
    }

    /**
     *  @brief Request a DMA/QDMA/Link channel.
     *
     *  Each channel (DMA/QDMA/Link) must be requested  before initiating a DMA
     *  transfer on that channel.
     *
     *  This API is used to allocate a logical channel (DMA/QDMA/Link) along with
     *  the associated resources. For DMA and QDMA channels, TCC and PaRAM Set are
     *  also allocated along with the requested channel.
     *
     *  User can request a specific logical channel by passing the channel number
     *  in 'ch_num'.
     *
     *  For DMA/QDMA channels, after allocating all the EDMA3 resources, this API
     *  sets the TCC field of the OPT PaRAM Word with the allocated TCC. It also sets
     *  the event queue for the channel allocated. The event queue needs to be
     *  specified by the user.
     *
     *  For DMA channel, it also sets the DCHMAP register.
     *
     *  For QDMA channel, it sets the QCHMAP register and CCNT as trigger word and
     *  enables the QDMA channel by writing to the QEESR register.
     *
     *  @param  ch_type                   (DMA/QDMA) Channel
     *                                    For Example: For DMA it is
     *                                    EDMA3_CHANNEL_TYPE_DMA.
     *
     *  @param  ch_num                    This is the channel number requested for a
     *                                   particular event.
     *
     *  @param  tcc_num                   The channel number on which the
     *                                   completion/error interrupt is generated.
     *                                   Not used if user requested for a Link
     *                                   channel.
     *
     *  @param  evt_Qnum                  Event Queue Number to which the channel
     *                                   will be mapped (valid only for the
     *                                   Master Channel (DMA/QDMA) request).
     *
     *  @return  TRUE if parameters are valid, else FALSE
     */
    bool request_channel(const e_EDMA3_CH_TYPE ch_type, const uint32_t ch_num, const uint32_t tcc_num, const e_EVENT_QUEUE evt_Qnum) noexcept
    {
        bool result = false;

        constexpr uint32_t OPT_TCC_MSK   = 0x0003F000u;
        constexpr uint32_t OPT_TCC_SHIFT = 0x0000000Cu;

        #define OPT_TCC_SET(tcc) (((OPT_TCC_MSK >> OPT_TCC_SHIFT) & (tcc)) << OPT_TCC_SHIFT)

        // TCC валиден в диапазоне всех каналов EDMA (0..63)
        if (tcc_num >= AM335X_DMACH_MAX)
        {
            return false;
        }

        if (CHANNEL_TYPE_DMA == ch_type && ch_num < AM335X_DMACH_MAX)
        {
            enable_ch_in_shadow_reg(ch_type, ch_num);
            map_ch_to_evtQ(ch_type, ch_num, evt_Qnum);

            enable_evt_intr(tcc_num); // Включаем прерывание по TCC!

            AM335X_EDMA3CC->OPT(ch_num) &= (~OPT_TCC_MSK);
            AM335X_EDMA3CC->OPT(ch_num) |= OPT_TCC_SET(tcc_num);
            result = true;
        }
        else if (CHANNEL_TYPE_QDMA == ch_type && ch_num < AM335X_QDMACH_MAX)
        {
            enable_ch_in_shadow_reg(ch_type, ch_num);
            map_ch_to_evtQ(ch_type, ch_num, evt_Qnum);

            enable_evt_intr(tcc_num); // Включаем прерывание по TCC!

            const uint32_t qdma_param_id = 32 + ch_num; // PaRAM 32..39 для QDMA
            AM335X_EDMA3CC->OPT(qdma_param_id) &= (~OPT_TCC_MSK);
            AM335X_EDMA3CC->OPT(qdma_param_id) |= OPT_TCC_SET(tcc_num);
            result = true;
        }

        return result;
    }

    /**
     *  @brief    Free the specified channel (DMA/QDMA/Link) and its associated
     *            resources (PaRAM Set, TCC etc) and removes various mappings.
     *
     *  For Link channels, this API only frees the associated PaRAM Set.
     *
     *  For DMA/QDMA channels, it does the following operations:
     *  1) Disable any ongoing transfer on the channel,
     *  2) Remove the channel to Event Queue mapping,
     *  3) For DMA channels, clear the DCHMAP register, if available
     *  4) For QDMA channels, clear the QCHMAP register,
     *  5) Frees the DMA/QDMA channel in the end.
     *
     *  @param  ch_type              (DMA/QDMA) Channel
     *                     For Example: For QDMA it is,
     *                     EDMA3_CHANNEL_TYPE_QDMA.
     *
     *  @param  ch_num                    This is the channel number requested for a
     *                      particular event.
     *
     *  @param  trig_mode                 Mode of triggering start of transfer.
     *
     *  @param  tcc_num                   The channel number on which the
     *                                   completion/error interrupt is generated.
     *                                   Not used if user requested for a Link
     *                                   channel.
     *
     *  @param  evt_Qnum                  Event Queue Number to which the channel
     *                                   will be unmapped (valid only for the
     *                                   Master Channel (DMA/QDMA) request).
     *
     *  trig_mode can have values:
     *        EDMA3_TRIG_MODE_MANUAL
     *        EDMA3_TRIG_MODE_QDMA
     *        EDMA3_TRIG_MODE_EVENT
     *
     *  @return  TRUE if parameters are valid else return FALSE
     */
    bool  free_channel(const e_EDMA3_CH_TYPE ch_type, const uint32_t ch_num, const uint32_t trig_mode, const uint32_t tcc_num, const e_EVENT_QUEUE evt_Qnum) noexcept
    {
        bool result = false;
        (void)evt_Qnum;

        if (ch_num <  AM335X_DMACH_MAX)
        {
            disable_transfer(ch_num, trig_mode);
            disable_ch_in_shadow_reg(ch_type, ch_num); // Also disable the DMA channel in the shadow region specific register
            unmap_ch_to_evtQ(ch_type, ch_num);

            if (CHANNEL_TYPE_DMA == ch_type)
            {
                // Interrupt channel nums are < 32
                if (tcc_num <  AM335X_DMACH_MAX)
                {
                    disable_evt_intr(ch_num);
                    result = true;
                }
            }
            else if (CHANNEL_TYPE_QDMA == ch_type)
            {
                // Interrupt channel nums are < 8
                if (tcc_num <  AM335X_QDMACH_MAX)
                {
                    disable_evt_intr(ch_num);
                    result = true;
                }
            }
        }

        return result;
    }

    /**
     *  @brief    Start EDMA transfer on the specified channel.
     *
     *  There are multiple ways to trigger an EDMA3 transfer. The triggering mode
     *  option allows choosing from the available triggering modes: Event,
     *  Manual or QDMA.
     *
     *  In event triggered, a peripheral or an externally generated event triggers
     *  the transfer. This API clears the Event and Event Miss Register and then
     *  enables the DMA channel by writing to the EESR.
     *
     *  In manual triggered mode, CPU manually triggers a transfer by writing a 1
     *  in the Event Set Register ESR. This API writes to the ESR to start the
     *  transfer.
     *
     *  In QDMA triggered mode, a QDMA transfer is triggered when a CPU (or other
     *  EDMA3 programmer) writes to the trigger word of the QDMA channel PaRAM set
     *  (auto-triggered) or when the EDMA3CC performs a link update on a PaRAM set
     *  that has been mapped to a QDMA channel (link triggered). This API enables
     *  the QDMA channel by writing to the QEESR register.
     *
     *  @param  ch_num           Channel being used to enable transfer.
     *
     *  @param  trig_mode        Mode of triggering start of transfer (Manual,
     *                          QDMA or Event).
     *
     *  trig_mode can have values:
     *        EDMA3_TRIG_MODE_MANUAL
     *        EDMA3_TRIG_MODE_QDMA
     *        EDMA3_TRIG_MODE_EVENT
     *
     *  @return  retVal         TRUE or FALSE depending on the param passed.
     *
     */
    bool  enable_transfer(const uint32_t ch_num, const uint32_t trig_mode) noexcept
    {
        bool result = false;

        switch (trig_mode)
        {
            case TRIG_MODE_MANUAL:
                if (ch_num <  AM335X_DMACH_MAX)
                {
                    set_event(ch_num);
                    result = true;
                }
               break;

            case TRIG_MODE_QDMA:
                if (ch_num <  AM335X_QDMACH_MAX)
                {
                    enable_QDMA_event(ch_num);
                    result = true;
                }
            break;

            case TRIG_MODE_EVENT:
                if (ch_num <  AM335X_DMACH_MAX)
                {
                    //clear SECR & EMCR to clean any previous NULL request
                    clr_miss_evt(ch_num);

                    // Set EESR to enable event
                    enable_DMA_event(ch_num);
                    result = true;
                }
            break;

            default :
                result = false;
            break;
        }

        return result;
    }

    /**
     *  @brief   Disable DMA transfer on the specified channel
     *
     *  There are multiple ways by which an EDMA3 transfer could be triggered.
     *  The triggering mode option allows choosing from the available triggering
     *  modes.
     *
     *  To disable a channel which was previously triggered in manual mode,
     *  this API clears the Secondary Event Register and Event Miss Register,
     *  if set, for the specific DMA channel.
     *
     *  To disable a channel which was previously triggered in QDMA mode, this
     *  API clears the QDMA Event Enable Register, for the specific QDMA channel.
     *
     *  To disable a channel which was previously triggered in event mode, this API
     *  clears the Event Enable Register, Event Register, Secondary Event Register
     *  and Event Miss Register, if set, for the specific DMA channel.
     *
     *
     *  @param  ch_num           Channel being used to enable transfer.
     *
     *  @param  trig_mode        Mode of triggering start of transfer (Manual,
     *                          QDMA or Event).
     *
     *  trig_mode can have values:
     *        EDMA3_TRIG_MODE_MANUAL
     *        EDMA3_TRIG_MODE_QDMA
     *        EDMA3_TRIG_MODE_EVENT
     *
     *  @return  retVal         TRUE or FALSE depending on the param passed.
     *
     */
    bool  disable_transfer(const uint32_t ch_num, const uint32_t trig_mode) noexcept
    {
        bool result = false;

        switch (trig_mode)
        {
            case TRIG_MODE_MANUAL:
                if (ch_num <  AM335X_DMACH_MAX)
                {
                    clr_event(ch_num);
                    result = true;
                }
            break;

            case TRIG_MODE_QDMA:
                if (ch_num <  AM335X_QDMACH_MAX)
                {
                    disable_QDMA_event(ch_num);
                    result = true;
                }
            break;

            case TRIG_MODE_EVENT:
                if (ch_num <  AM335X_DMACH_MAX)
                {
                    // clear SECR & EMCR to clean any previous NULL request
                    clr_miss_evt(ch_num);

                    // Set EESR to enable event
                    disable_DMA_event(ch_num);
                    result = true;
                }
            break;

            default :
                result = false;
            break;
        }

        return result;
    }

    /**
     *  @brief  Clears Event Register and Error Register for a specific
     *          DMA channel and brings back EDMA3 to its initial state.
     *
     *  This API clears the Event register, Event Miss register, Event Enable
     *  register for a specific DMA channel. It also clears the CC Error register.
     *
     *  @param  ch_num           This is the channel number requested for a
     *                          particular event.
     *
     *  @param  evt_Qnum         Event Queue Number to which the channel
     *                          will be unmapped (valid only for the
     *                          Master Channel (DMA/QDMA) request).
     *
     *  @return none.
     */
    void clear_error_bits(const uint32_t ch_num, const e_EVENT_QUEUE evt_Qnum) noexcept
    {
        auto& cc = *AM335X_EDMA3CC;
        constexpr uint32_t CCERRCLR_TCCERR    = 0x00010000u;
        constexpr uint32_t CCERRCLR_QTHRXCD2  = 0x00000004u;
        constexpr uint32_t CCERRCLR_QTHRXCD1  = 0x00000002u;
        constexpr uint32_t CCERRCLR_QTHRXCD0  = 0x00000001u;

        if(ch_num <  AM335X_DMACH_MAX)
        {
             if(ch_num < 32)
             {
                  cc.S_EECR(region_id).reg = (0x01u << ch_num);
                  cc.EMCR.reg = (0x01u << ch_num);                  // Write to EMCR to clear the corresponding EMR bit
                  cc.S_SECR(region_id).reg = (0x01u << ch_num);     // Clears the SER

             }
             else
             {
                  cc.S_EECRH(region_id).reg = (0x01u << (ch_num - 32));
                  cc.EMCRH.reg = (0x01u << (ch_num - 32));                // Write to EMCRH to clear the corresponding EMR bi
                  cc.S_SECRH(region_id).reg = (0x01u << (ch_num - 32));   // Clears the SER
             }
        }

        switch(evt_Qnum)
        {
            case  EVENT_Q0:
              cc.CCERRCLR.reg &= (CCERRCLR_QTHRXCD0 |CCERRCLR_TCCERR);
              break;
            case  EVENT_Q1:
              cc.CCERRCLR.reg &= (CCERRCLR_QTHRXCD1 |CCERRCLR_TCCERR);
              break;
            case  EVENT_Q2:
              cc.CCERRCLR.reg &= (CCERRCLR_QTHRXCD2|CCERRCLR_TCCERR);
              break;
            default:
                cc.CCERRCLR.reg &= (CCERRCLR_QTHRXCD2|CCERRCLR_TCCERR);
              break;
        }
    }

    /**
     *  @brief   This returns EDMA3 CC error status.
     *
     *  @return  value                  Status of the Interrupt Pending Register
     *
     */
    uint32_t get_CC_Err_status() noexcept
    {
        uint32_t intr_status_val = 0;

        intr_status_val = AM335X_EDMA3CC->CCERR.reg;

        return intr_status_val;
    }

    /**
     *  @brief   This returns error interrupt status for those events whose
     *           event number is less than 32.
     *
     *  @return  value                  Status of the Interrupt Pending Register
     *
     */
    uint32_t get_Err_intr_status() noexcept
    {
        uint32_t intr_status_val = 0;

        intr_status_val = AM335X_EDMA3CC->EMR.reg;

        return intr_status_val;
    }

    /**
     *  @brief   This returns QDMA error interrupt status.
     *
     *  @return  value              Status of the QDMA Interrupt Pending Register
     *
     */
    uint32_t QDMA_get_Err_intr_status() noexcept
    {
        uint32_t intr_status_val = 0;

        intr_status_val = AM335X_EDMA3CC->QEMR.reg;

        return intr_status_val;
    }

    /**
     *  @brief   This API enables to evaluate the subsequent errors. On writing
     *           to the EEVAL register EDMA3CC error interrupt will be reasserted,
     *           if there are any outstanding error bits set due to subsequent
     *           error conditions.
     *
     */
    void CC_Err_evaluate() noexcept
    {
        constexpr uint32_t EEVAL_EVAL        = 0x00000001u;
        constexpr uint32_t EEVAL_EVAL_SHIFT  = 0x00000000u;
        AM335X_EDMA3CC->EEVAL.reg = EEVAL_EVAL << EEVAL_EVAL_SHIFT;
    }

    /**
     *  @brief   EDMA3 Deinitialization
     *
     *  This function deinitializes the EDMA3 Driver
     *  Clears the error specific registers (EMCR/EMCRh, QEMCR, CCERRCLR) &
     *  deinitialize the Queue Number Registers
     *
     *  @param   que_num          Event Queue used
     *
     *  Note :     The region_id is the shadow region(0 or 1) used and the,
     *            Event Queue used is either (0 or 1). There are only two shadow regions
     *            and only two event Queues
     */
    void deinit(const uint32_t que_num) noexcept
    {
        auto& cc = *AM335X_EDMA3CC;
        (void)que_num;

        uint32_t count = 0;
        constexpr uint32_t CCERRCLR_TCCERR = 0x00010000u;


        // Disable the DMA (0 - 62) channels in the DRAE register
        cc.DRAE(region_id).reg =  CLR_ALL_BITS;
        cc.DRAEH(region_id).reg =  CLR_ALL_BITS;

        clr_CC_Err(CCERRCLR_TCCERR);

        // Clear the Event miss Registers
        cc.EMCR.reg =  SET_ALL_BITS;
        cc.EMCRH.reg =  SET_ALL_BITS;
        // Clear CCERR register
        cc.CCERRCLR.reg =  SET_ALL_BITS;

        // Deinitialize the Queue Number Registers
        for (count = 0;count <  AM335X_DMACH_MAX; count++)
        {
            cc.DMAQNUM[count >> 3u].reg &=  DMAQNUM_CLR(count);
        }

        for (count = 0;count <  AM335X_QDMACH_MAX; count++)
        {
            cc.QDMAQNUM.reg &=  QDMAQNUM_CLR(count);
        }
    }

    /**
     * @brief   This API return the revision Id of the peripheral.
     */
    uint32_t peripheral_id_get() noexcept
    {
        return AM335X_EDMA3CC->PID.reg;
    }

    /**
     *  @brief   This function returns interrupts status of those events
     *           which is greater than 32.
     *
     *  @return  value                  Status of the Interrupt Pending Register
     *
     */
    uint32_t intr_status_high_get() noexcept
    {
        uint32_t intr_sts_val = 0;

        intr_sts_val = AM335X_EDMA3CC->S_IPRH(region_id).reg;

        return intr_sts_val;
    }

    /**
     *  @brief   This returns error interrupt status for those events whose
     *           event number is greater than 32.
     *
     *  @return  value                  Status of the Interrupt Pending Register
     *
     */
    uint32_t Err_intr_high_status_get() noexcept
    {
        uint32_t intr_sts_val = 0;

        intr_sts_val = AM335X_EDMA3CC->EMRH.reg;

        return intr_sts_val;
    }

    /**
     *  @brief   This function maps DMA channel to any of the PaRAM sets
     *           in the PaRAM memory map.
     *
     *  @param   channel   The DMA channel number required to be mapped.
     *
     *  @param   param_set  It specifies the param_set to which DMA channel
     *                     required to be mapped.
     */
    void channel_to_param_map(const uint32_t channel, const uint32_t param_set) noexcept
    {
        AM335X_EDMA3CC->DCHMAP[channel].reg = param_set << 5;
    }

    /**
     *  @brief   This API can be used to save the register context for EDMA
     *
     *  @param  p_edma_cntx     Pointer to the structure where the context
     *                          needs to be saved.
     */
    void context_save(EDMACONTEXT_t *p_edma_cntx) noexcept
    {
        const auto& cc = *AM335X_EDMA3CC;

        uint32_t i;
        uint32_t max_par = 128;

        // Get the Channel mapping reg Val
        for(i = 0; i <  AM335X_DMACH_MAX; i++)
        {
             p_edma_cntx->dch_map[i] = cc.DCHMAP[i].reg;
        }

        // Get DMA Queue Number Register Val
        for(i = 0; i <  AM335X_QDMACH_MAX; i++)
        {
            p_edma_cntx->dma_Qnum[i] = cc.DMAQNUM[i].reg;
        }

        // Get the DMA Region Access Enable Register val
        p_edma_cntx->reg_acc_enable_low =  cc.DRAE(static_cast<e_REGION_ID>(0)).reg;
        p_edma_cntx->reg_acc_enable_high =  cc.DRAEH(static_cast<e_REGION_ID>(0)).reg;

        // Get Event Set Register value
        p_edma_cntx->event_set_reg_low  =  cc.S_ESR(static_cast<e_REGION_ID>(0)).reg;
        p_edma_cntx->event_set_reg_high =  cc.S_ESRH(static_cast<e_REGION_ID>(0)).reg;

        // Get Event Enable Set Register value
        p_edma_cntx->enable_evt_set_reg_low =   cc.S_EER(static_cast<e_REGION_ID>(0)).reg;
        p_edma_cntx->enable_evt_set_reg_high =  cc.S_EERH(static_cast<e_REGION_ID>(0)).reg;

        // Get Interrupt Enable Set Register value
        p_edma_cntx->int_enable_set_reg_low  =   cc.S_IER(static_cast<e_REGION_ID>(0)).reg;
        p_edma_cntx->int_enable_set_reg_high =   cc.S_IERH(static_cast<e_REGION_ID>(0)).reg;

        if(EDMA_REVID == version_get())
        {
            max_par = 256;
        }

        for(i = 0; i < max_par; i++)
        {
            get_paRAM(i,(p_edma_cntx->dma_par_entry[i]));
        }
    }

    /**
     *  @brief   This API can be used to restore the register context for EDMA
     *
     *  @param  p_edma_cntx     Pointer to the structure where the context
     *                          needs to be restored from.
     */
    void context_restore(const EDMACONTEXT_t *p_edma_cntx) noexcept
    {
        auto& cc = *AM335X_EDMA3CC;
        uint32_t i;
        uint32_t max_par = 128;

        // set the Channel mapping reg Val
        for(i = 0; i <  AM335X_DMACH_MAX; i++)
        {
             // All events are one to one mapped with the channels
             cc.DCHMAP[i].reg = p_edma_cntx->dch_map[i];
        }

        // set DMA Queue Number Register Val
        for(i  =0; i <  AM335X_QDMACH_MAX; i++)
        {
            cc.DMAQNUM[i].reg = p_edma_cntx->dma_Qnum[i];
        }

        // set the DMA Region Access Enable Register val
         cc.DRAE(region_id).reg = p_edma_cntx->reg_acc_enable_low;
         cc.DRAEH(region_id).reg = p_edma_cntx->reg_acc_enable_high;

        // set Event Set Register value
         cc.S_ESR(region_id).reg = p_edma_cntx->event_set_reg_low;
         cc.S_ESRH(region_id).reg = p_edma_cntx->event_set_reg_high;

        // set Event Enable Set Register value
         cc.S_EER(region_id).reg = p_edma_cntx->enable_evt_set_reg_low;
         cc.S_EERH(region_id).reg = p_edma_cntx->enable_evt_set_reg_high;

        // set Interrupt Enable Set Register value
         cc.S_IER(region_id).reg = p_edma_cntx->int_enable_set_reg_low;
         cc.S_IERH(region_id).reg = p_edma_cntx->int_enable_set_reg_high;

        if( EDMA_REVID == version_get())
        {
            max_par = 256;
        }

        for(i = 0; i < max_par; i++)
        {
            set_paRAM(i,p_edma_cntx->dma_par_entry[i]);
        }
    }
}