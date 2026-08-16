#include "hal/EDMA/EDMA_diagnostics.hpp"
#include "regs/EDMA.hpp"
#include <cstdio>
#include <cstring>

#include "rtt/rtt_log.h"

#define TAG "EDMA_DIAG"

namespace HAL::EDMA
{
    template <typename T = int32_t>
    [[nodiscard]] constexpr int32_t set_queue(T q) noexcept
    {
        return (static_cast<int32_t>(q) == -1) ? 0xFFFFFFFFU : static_cast<int32_t>(q);
    }

    namespace
    {
        EDMA_Channel_Diagnostic capture_DMA_ch(const uint32_t ch) noexcept
        {
            using namespace REGS::EDMA;
            const auto cc = AM335X_EDMA3CC;
            EDMA_Channel_Diagnostic d{};

            d.channel = ch;
            d.is_qdma = false;
            d.param_id = cc->DCHMAP[ch].b.PAENTRY;
            d.param = cc->paRAM(d.param_id);
            d.dchmap_or_qchmap = cc->DCHMAP[ch].reg;
            d.param_opt = d.param.OPT.reg;
            d.tcc = d.param.OPT.b.TCC;
            d.queue = static_cast<int32_t>(get_queue_for_DMA_channel(ch));
            d.queue = (d.queue == static_cast<int32_t>(0xFFFFFFFF)) ? -1 : static_cast<unsigned>(d.queue);

            e_REGION_ID r_id = get_region_id();
            const uint32_t bit = 1u << (ch & 31u);
            const bool hi = ch >= 32;
            const uint32_t er  = (hi) ? cc->S_ERH(r_id).reg : cc->S_ER(r_id).reg;
            const uint32_t eer = (hi) ? cc->S_EERH(r_id).reg : cc->S_EER(r_id).reg;
            const uint32_t ser = (hi) ? cc->S_SERH(r_id).reg : cc->S_SER(r_id).reg;
            const uint32_t ipr = (hi) ? cc->S_IPRH(r_id).reg : cc->S_IPR(r_id).reg;
            const uint32_t ier = (hi) ? cc->S_IERH(r_id).reg : cc->S_IER(r_id).reg;
            const uint32_t cer = (hi) ? cc->S_CERH(r_id).reg : cc->S_CER(r_id).reg;
            const uint32_t access = (hi) ? cc->DRAEH(r_id).reg : cc->DRAE(r_id).reg;

            d.event = (er & bit) != 0;
            d.event_enable = (eer & bit) != 0;
            d.secondary_event = (ser & bit) != 0;
            d.interrupt_pending = (ipr & bit) != 0;
            d.interrupt_enable = (ier & bit) != 0;
            d.chained_event = (cer & bit) != 0;
            d.shadow_access = (access & bit) != 0;

            return d;
        }

        EDMA_Channel_Diagnostic capture_QDMA_ch(const uint32_t ch) noexcept
        {
            using namespace REGS::EDMA;
            const auto& cc = *AM335X_EDMA3CC;
            EDMA_Channel_Diagnostic d{};

            d.channel = ch;
            d.is_qdma = true;
            d.dchmap_or_qchmap = cc.QCHMAP[ch].reg;
            d.param_id = cc.QCHMAP[ch].b.PAENTRY;
            d.param = cc.paRAM(d.param_id);
            d.param_opt = d.param.OPT.reg;
            d.tcc = d.param.OPT.b.TCC;
            d.queue = set_queue<>(get_queue_for_QDMA_channel(ch));

            const e_REGION_ID r_id = get_region_id();
            const uint32_t bit = 1u << ch;
            d.event = (cc.S_QER(r_id).reg & bit) != 0;
            d.event_enable = (cc.S_QEER(r_id).reg & bit) != 0;
            d.secondary_event = (cc.S_QSER(r_id).reg & bit) != 0;
            d.shadow_access = (cc.QRAE[r_id].reg & bit) != 0;

            return d;
        }
    }



    EDMA_TC_Diagnostic EDMA_Diagnostics::captureTC(const uint32_t tc_idx) noexcept
    {
        using namespace REGS::EDMA;
        EDMA_TC_Diagnostic d{};
        if (tc_idx >= AM335x_TCS_MAX) return d;

        const auto& tc = *getTC(tc_idx);
        d.base = reinterpret_cast<uint32_t>(getTC(tc_idx));
        d.pid = tc.PID.reg;
        d.tccfg = tc.TCCFG.reg;
        d.sysconfig = tc.SYSCONFIG.reg;
        d.tcstat = tc.TCSTAT.reg;
        d.errstat = tc.ERRSTAT.reg;
        d.erren = tc.ERREN.reg;
        d.errdet = tc.ERRDET.reg;
        d.rdrate = tc.RDRATE.reg;

        d.saopt = tc.SAOPT.reg;
        d.sasrc = tc.SASRC.reg;
        d.sacnt = tc.SACNT.reg;
        d.sadst = tc.SADST.reg;

        // Decode TI error struct state
        d.errors.bus_error  = tc.ERRSTAT.b.BUSERR;
        d.errors.tr_error   = tc.ERRSTAT.b.TRERR;
        d.errors.mmra_error = tc.ERRSTAT.b.MMRAERR;

        if (d.errors.mmra_error)
        {
            d.errors.stat    = tc.ERRDET.b.STAT;
            d.errors.tcc     = tc.ERRDET.b.TCC;
            d.errors.tcinten = tc.ERRDET.b.TCINTEN;
            d.errors.tcchen  = tc.ERRDET.b.TCCHEN;
        }

        // Active State
        d.tcstat_dfstrtptr = tc.TCSTAT.b.DFSTRTPTR;
        d.tcstat_wsactive  = tc.TCSTAT.b.WSACTV;
        d.tcstat_progbusy  = tc.TCSTAT.b.PROGBUSY;
        d.tcstat_dstactv   = tc.TCSTAT.b.DSTACTV;
        d.tcstat_srcactive = tc.TCSTAT.b.SRCACTV;

        return d;
    }

    EDMA_CC_Diagnostic EDMA_Diagnostics::captureCC() noexcept
    {
        using namespace REGS::EDMA;
        EDMA_CC_Diagnostic d{};
        const auto& cc = *AM335X_EDMA3CC;

        d.base = reinterpret_cast<uint32_t>(AM335X_EDMA3CC);
        d.pid = cc.PID.reg;
        d.cccfg = cc.CCCFG.reg;
        d.sysconfig = cc.SYSCONFIG.reg;

        // Capture DCHMAP & QCHMAP
        for (size_t i = 0; i < AM335X_DMACH_MAX; ++i) d.dchmap[i] = cc.DCHMAP[i].reg;
        for (size_t i = 0; i < AM335X_QDMACH_MAX; ++i)  d.qchmap[i] = cc.QCHMAP[i].reg;

        // Capture Queue assignment
        for (size_t i = 0; i < 8; ++i) d.dma_qnum[i] = cc.DMAQNUM[i].reg;
        d.qdma_qnum = cc.QDMAQNUM.reg;
        d.quepri    = cc.QUEPRI.reg;

        d.qstat[0] = cc.QSTAT_0.reg;
        d.qstat[1] = cc.QSTAT_1.reg;
        d.qstat[2] = cc.QSTAT_2.reg;

        // Event / Missed / Errors
        d.emr   = cc.EMR.reg;
        d.emrh  = cc.EMRH.reg;
        d.qemr  = cc.QEMR.reg;
        d.ccerr = cc.CCERR.reg;

        const volatile DRAE_reg_t* drae = &cc.DRAE0;
        const volatile DRAEH_reg_t* draeh = &cc.DRAEH0;
        for (size_t r = 0; r < AM335x_REGIONS_MAX; ++r)
        {
            d.drae[r]  = drae->reg;
            d.draeh[r] = draeh->reg;
            d.qrae[r]  = cc.QRAE[r].reg;

            drae++;
            draeh++;
        }

        return d;
    }

    void EDMA_Diagnostics::capture(EDMA_DiagnosticSnapshot *snapshot) noexcept
    {
        using namespace REGS::EDMA;
        snapshot->region_id = static_cast<uint32_t>(get_region_id());
        if (snapshot->region_id >= AM335x_REGIONS_MAX) snapshot->region_id = 0;
        snapshot->cc = captureCC();
        for (uint32_t i = 0; i < AM335x_TCS_MAX; ++i) snapshot->tc[i] = captureTC(i);
        for (uint32_t i = 0; i < AM335X_DMACH_MAX; ++i) snapshot->dma[i] = capture_DMA_ch(i);
        for (uint32_t i = 0; i < AM335X_QDMACH_MAX; ++i) snapshot->qdma[i] = capture_QDMA_ch(i);
    }

    void EDMA_Diagnostics::dump_full_diagnostics(EDMA_DiagnosticSnapshot &s,
                                                 const uint8_t channel,
                                                 const bool is_qdma,
                                                 const char* reason) noexcept
    {
        using namespace HAL::EDMA;

        capture(&s);

        RTT_LOG_E(TAG, "=== EDMA DIAGNOSTIC DUMP [%s] (%s CH %u) ===",
                  reason, is_qdma ? "QDMA" : "DMA", channel);

        decodeChannel(s, channel, is_qdma);
        decodeCC(s);

        findChannelByTCC(s, channel);

        for (uint8_t tc = 0; tc < REGS::EDMA::AM335x_TCS_MAX; ++tc)
        {
            decodeTC(s, channel, tc, is_qdma);
        }

        clearCCErrors(0xFFFFFFFF);
        for (uint8_t tc = 0; tc < REGS::EDMA::AM335x_TCS_MAX; ++tc)
        {
            clearTCError(tc, 0xFFFFFFFF);
        }
        RTT_LOG_E(TAG, "===============================================");
    }

    void EDMA_Diagnostics::decodeCC(const EDMA_DiagnosticSnapshot& s) noexcept
    {
        const EDMA_CC_Diagnostic& d = s.cc;
        RTT_LOG_I("CC_STAT","  EMR   :  EMRH  :  QEMR  : CCERR  :  EEVAL {          QSTAT           } CCSTAT : MPFAR  :  MPFSR");
        RTT_LOG_I("CC_STAT","%08X:%08X:%08X:%08X:%08X{%08X,%08X,%08X}%08X:%08X:%08X", static_cast<unsigned>(d.emr),
                                                                                        static_cast<unsigned>(d.emrh),
                                                                                        static_cast<unsigned>(d.qemr),
                                                                                        static_cast<unsigned>(d.ccerr),
                                                                                        static_cast<unsigned>(d.eeval),
                                                                                        static_cast<unsigned>(d.qstat[0]),
                                                                                        static_cast<unsigned>(d.qstat[1]),
                                                                                        static_cast<unsigned>(d.qstat[2]),
                                                                                        static_cast<unsigned>(d.ccstat),
                                                                                        static_cast<unsigned>(d.mpfar),
                                                                                        static_cast<unsigned>(d.mpfsr));
    }

    void EDMA_Diagnostics::decodeTC(const EDMA_DiagnosticSnapshot& s,
                                      const uint32_t channel,
                                      const uint8_t tc_idx ,
                                      const bool is_qdma) noexcept
    {
        const auto str = "TC";
        char tc_name[10];
        std::snprintf(tc_name, sizeof(tc_name), "%s%u_DIAG", str, tc_idx);

        const EDMA_TC_Diagnostic & d = s.tc[tc_idx];
        const EDMA_Channel_Diagnostic & cd = is_qdma ? s.qdma[channel] : s.dma[channel];
        const char* stat = d.errstat == 0 ? "no err" : (d.errors.mmra_error ? "rd err" : "wr err");
        RTT_LOG_I(tc_name, "----------------------------------------------------------------------");
        RTT_LOG_I("ERROR","ERRSTAT :[BUSERR:TRERR:MMRAERR]: ERRDET :[   STAT():TCC:TCINTEN:TCCHEN]");
        RTT_LOG_I("ERROR","%08X:[   %u   :  %u  :   %u  ]:%08X:[%X(%s): %u :   %u   :   %u  ]",(unsigned)d.errstat,
                                                                     (unsigned)d.errors.bus_error,
                                                                     (unsigned)d.errors.tr_error,
                                                                     (unsigned)d.errors.mmra_error,
                                                                     (unsigned)d.errdet,
                                                                     (unsigned)d.errors.stat,
                                                                     stat,
                                                                     (unsigned)d.errors.tcc,
                                                                     (unsigned)d.errors.tcinten,
                                                                     (unsigned)d.errors.tcchen);
        RTT_LOG_I("STAT"," TCSTAT [PROGBUSY:SRCACTV:WSACTV:DSTACTV:DFSTRTPTR]");
        RTT_LOG_I("STAT","%08X[    %u   :   %u   :   %u  :   %u   :    %u    ]",(unsigned)d.tcstat,
                                                    (unsigned)d.tcstat_progbusy,
                                                    (unsigned)d.tcstat_srcactive,
                                                    (unsigned)d.tcstat_wsactive,
                                                    (unsigned)d.tcstat_dstactv,
                                                    (unsigned)d.tcstat_dfstrtptr);
        RTT_LOG_I("ACTIVE","  SRC   :   DST  :  SACNT [TCC:TCINTEN:TCCHEN]");
        RTT_LOG_I("ACTIVE","%08X:%08X:%08X[ %u :   %u   :  %u   ]",    (unsigned)d.sasrc,
                                                       (unsigned)d.sadst,
                                                       (unsigned)d.sacnt,
                                                       (unsigned)cd.tcc,
                                                       (unsigned)cd.param.OPT.b.TCINTEN,
                                                       (unsigned)cd.param.OPT.b.TCCHEN);


    }

    void EDMA_Diagnostics::decodeChannel(const EDMA_DiagnosticSnapshot& s, const uint32_t channel, const bool is_qdma) noexcept
    {
        const EDMA_Channel_Diagnostic & d = is_qdma ? s.qdma[channel] : s.dma[channel];
        const auto str = "CH";
        char ch_stat[8];
        std::snprintf(ch_stat, sizeof(ch_stat), "%s%u_STAT", str, (unsigned)channel);
        RTT_LOG_I(ch_stat,"TYPE:CH:PaRAM:TCC:QUEUE:   OPT  [EV:EER:SER:IPR:IER:CER:SH_ACCESS]");
        RTT_LOG_I(ch_stat,"%s: %u:  %u : %u :  %d  :%08X[ %u: %u : %u : %u : %u : %u :    %u    ]",  d.is_qdma ? "QDMA" : "DMA ",
                                                                        (unsigned)d.channel,
                                                                        (unsigned)d.param_id,
                                                                        (unsigned)d.tcc,
                                                                        (int)set_queue<>(d.queue),
                                                                        (unsigned)d.param_opt,
                                                                        (unsigned)d.event,
                                                                        (unsigned)d.event_enable,
                                                                        (unsigned)d.secondary_event,
                                                                        (unsigned)d.interrupt_pending,
                                                                        (unsigned)d.interrupt_enable,
                                                                        (unsigned)d.chained_event,
                                                                        (unsigned)d.shadow_access);
    }

    void EDMA_Diagnostics::findChannelByTCC(const EDMA_DiagnosticSnapshot& s, const uint32_t tcc) noexcept
    {
        using namespace REGS::EDMA;
        EDMA_TCC_Path_Trace trace{};
        trace.target_tcc = tcc;

        // First search among the QDMAs
        for (size_t qch = 0; qch < AM335X_QDMACH_MAX; ++qch)
        {
            const auto& qdma_diag = s.qdma[qch];
            if (qdma_diag.event_enable && qdma_diag.tcc == tcc)
            {
                trace.found_qdma_channel = static_cast<int32_t>(qch);
                trace.param_id = static_cast<int32_t>(qdma_diag.param_id);
                trace.is_tcc_channel_matching = (qch == tcc);
                trace.mapped_queue = set_queue<>(qdma_diag.queue);
                trace.is_param_valid = true;
                break;
            }
        }

        // If no active ones are found in QDMA - search in DMA
        if (!trace.is_param_valid)
        {
            for (size_t ch = 0; ch < AM335X_DMACH_MAX; ++ch)
            {
                const auto& dma_diag = s.dma[ch];
                if (dma_diag.tcc == tcc)
                {
                    trace.found_dma_channel = static_cast<int32_t>(ch);
                    trace.param_id = static_cast<int32_t>(dma_diag.param_id);
                    trace.is_tcc_channel_matching = (ch == tcc);
                    trace.mapped_queue = set_queue<>(dma_diag.queue);
                    trace.is_param_valid = true;
                    break;
                }
            }
        }

        RTT_LOG_I("CH_BY_TCC", "TCC->Mapped Queue:PaRAM:Channel Match");
        RTT_LOG_I("CH_BY_TCC", " %u :       %u     :  %d :     %s    ",
                  static_cast<unsigned>(tcc),
                  (int)set_queue<>(trace.mapped_queue),
                   static_cast<int>(trace.param_id),
                  trace.is_tcc_channel_matching ? "YES" : "NO");
    }


    int32_t EDMA_Diagnostics::findParamByAddress(const uint32_t address) noexcept
    {
        if (address < REGS::EDMA::PARAM_BASE || address >= (REGS::EDMA::PARAM_BASE + 0x2000))
        {
            return -1;
        }
        return static_cast<int32_t>((address - REGS::EDMA::PARAM_BASE) / 0x20);
    }

    bool EDMA_Diagnostics::diagnoseChannel(const EDMA_DiagnosticSnapshot& s, const uint32_t channel, const bool is_qdma, EDMA_Channel_Diagnostic& out_diag) noexcept
    {
        if (!is_qdma && channel >= REGS::EDMA::AM335X_DMACH_MAX) return false;
        if (is_qdma && channel >= REGS::EDMA::AM335X_QDMACH_MAX) return false;

        out_diag = is_qdma ? s.qdma[channel] : s.dma[channel];

        return true;
    }

    void EDMA_Diagnostics::clearTCError(const uint32_t tc_idx, const uint32_t mask) noexcept
    {
        using namespace REGS::EDMA;
        if (tc_idx >= AM335x_TCS_MAX) return;

        auto& tc = *getTC(tc_idx);
        tc.ERRCLR.reg = mask;
    }

    void EDMA_Diagnostics::clearCCErrors(const uint32_t mask) noexcept
    {
        auto& cc = *REGS::EDMA::AM335X_EDMA3CC;
        cc.CCERRCLR.reg = mask;
    }
}
