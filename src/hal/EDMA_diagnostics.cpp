#include "hal/EDMA/EDMA_diagnostics.hpp"
#include "regs/EDMA.hpp"
#include <cstdio>
#include <cstring>

namespace HAL::EDMA
{
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
            d.queue = get_queue_for_DMA_channel(ch);

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
            d.dchmap_or_qchmap = cc.QDMAQNUM.reg;
            d.param_id = cc.QCHMAP[ch].b.PAENTRY;
            d.param = cc.paRAM(d.param_id);
            d.param_opt = d.param.OPT.reg;
            d.tcc = d.param.OPT.b.TCC;
            d.queue = get_queue_for_QDMA_channel(ch);

            const e_REGION_ID r_id = get_region_id();
            const uint32_t bit = 1u << ch;
            d.event = (cc.S_QER(r_id).reg & bit) != 0;
            d.event_enable = (cc.S_QEER(r_id).reg & bit) != 0;
            d.secondary_event = (cc.S_QSER(r_id).reg & bit) != 0;
            d.shadow_access = (cc.QRAE[ch].reg & bit) != 0;

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

        // Event / Missed / Errors
        d.emr   = cc.EMR.reg;
        d.emrh  = cc.EMRH.reg;
        d.qemr  = cc.QEMR.reg;
        d.ccerr = cc.CCERR.reg;

        return d;
    }

    void EDMA_Diagnostics::capture(EDMA_DiagnosticSnapshot *snapshot) noexcept
    {
        snapshot->region_id = static_cast<uint32_t>(get_region_id());
        if (snapshot->region_id >= REGS::EDMA::REGIONS_MAX) snapshot->region_id = 0;
        snapshot->cc = captureCC();
        //for (uint32_t i = 0; i < TCS; ++i) s.tc[i] = captureTC(i);
        //for (uint32_t i = 0; i < DMA_CHANNELS; ++i) s.dma[i] = capture_DMA_ch(i);
        //for (uint32_t i = 0; i < QDMA_CHANNELS; ++i) s.qdma[i] = capture_QDMA_ch(i);
    }

    size_t EDMA_Diagnostics::decodeCC(const EDMA_DiagnosticSnapshot& s, char* buf, size_t max_len) noexcept
    {
        const EDMA_CC_Diagnostic& d = s.cc;
        int written = std::snprintf(buf, max_len,
            "EMR=%08X EMRH=%08X QEMR=%08X CCERR=%08X EEVAL=%08X; "
            "QSTAT={%08X,%08X,%08X} CCSTAT=%08X MPFAR=%08X MPFSR=%08X",
            (unsigned)d.emr, (unsigned)d.emrh, (unsigned)d.qemr, (unsigned)d.ccerr, (unsigned)d.eeval,
            (unsigned)d.qstat[0], (unsigned)d.qstat[1], (unsigned)d.qstat[2], (unsigned)d.ccstat, (unsigned)d.mpfar, (unsigned)d.mpfsr);
        return (written > 0) ? static_cast<size_t>(written) : 0;
    }

    size_t EDMA_Diagnostics::decodeTC(const EDMA_DiagnosticSnapshot& s,
                                      char* buf,
                                      size_t max_len,
                                      uint32_t channel,
                                      uint8_t tc_idx ,
                                      bool is_qdma) noexcept
    {
        if (!buf || max_len == 0) return 0;

        const EDMA_TC_Diagnostic & d = s.tc[tc_idx];
        const EDMA_Channel_Diagnostic & cd = is_qdma ? s.qdma[channel] : s.dma[channel];
        const char* stat = d.errstat == 0 ? "no transaction error" :
                           (d.errors.mmra_error ? "read error" : "write error");
        int written = std::snprintf(buf, max_len,
            "ERRSTAT=%08X [BUSERR=%u TRERR=%u MMRAERR=%u], ERRDET=%08X [STAT=%X (%s), TCC=%u, TCINTEN=%u, TCCHEN=%u]; "
            "TCSTAT=%08X [PROGBUSY=%u SRCACTV=%u WSACTV=%u DSTACTV=%u DFSTRTPTR=%u]; "
            "ACTIVE: SRC=%08X DST=%08X SACNT=%08X TCC=%u TCINTEN=%u TCCHEN=%u",
            (unsigned)d.errstat, (unsigned)d.errors.bus_error, (unsigned)d.errors.tr_error, (unsigned)d.errors.mmra_error, (unsigned)d.errdet, (unsigned)d.errors.stat, stat,
            (unsigned)d.errors.tcc, (unsigned)d.errors.tcinten, (unsigned)d.errors.tcchen, (unsigned)d.tcstat,
            (unsigned)d.tcstat_progbusy, (unsigned)d.tcstat_srcactive, (unsigned)d.tcstat_wsactive, (unsigned)d.tcstat_dstactv,
            (unsigned)d.tcstat_dfstrtptr, (unsigned)d.sasrc, (unsigned)d.sadst, (unsigned)d.sacnt, (unsigned)cd.tcc,
            (unsigned)cd.param.OPT.b.TCINTEN, (unsigned)cd.param.OPT.b.TCCHEN);

        return (written > 0) ? static_cast<size_t>(written) : 0;
    }

    size_t EDMA_Diagnostics::decodeChannel(const EDMA_DiagnosticSnapshot& s, char* buf, size_t max_len, uint32_t channel, bool is_qdma) noexcept
    {
        const EDMA_Channel_Diagnostic & d = is_qdma ? s.qdma[channel] : s.dma[channel];
        int written = std::snprintf(buf, max_len,
            "%s CH=%u PaRAM=%u TCC=%u Q=%u OPT=%08X [EV=%u EER=%u SER=%u IPR=%u IER=%u CER=%u ACCESS=%u]",
            d.is_qdma ? "QDMA" : "DMA", (unsigned)d.channel, (unsigned)d.param_id, (unsigned)d.tcc, (unsigned)d.queue, (unsigned)d.param_opt,
            (unsigned)d.event, (unsigned)d.event_enable, (unsigned)d.secondary_event, (unsigned)d.interrupt_pending,
            (unsigned)d.interrupt_enable, (unsigned)d.chained_event, (unsigned)d.shadow_access);

        return (written > 0) ? static_cast<size_t>(written) : 0;
    }

    EDMA_TCC_Path_Trace EDMA_Diagnostics::findChannelByTCC(const EDMA_DiagnosticSnapshot& s, const uint32_t tcc) noexcept
    {
        using namespace REGS::EDMA;
        EDMA_TCC_Path_Trace trace{};
        trace.target_tcc = tcc;

        for (size_t ch = 0; ch < AM335X_DMACH_MAX; ++ch)
        {
            const auto& dma_diag = s.dma[ch];

            if (dma_diag.tcc == tcc)
            {
                trace.found_dma_channel = static_cast<int32_t>(ch);
                trace.param_id = static_cast<int32_t>(dma_diag.param_id);
                trace.is_tcc_channel_matching = (ch == tcc);
                trace.mapped_queue = dma_diag.queue;
                trace.is_param_valid = true;
                return trace;
            }
        }

        for (size_t qch = 0; qch < AM335X_QDMACH_MAX; ++qch)
        {
            const auto& qdma_diag = s.qdma[qch];

            if (qdma_diag.tcc == tcc)
            {
                trace.found_qdma_channel = static_cast<int32_t>(qch);
                trace.param_id = static_cast<int32_t>(qdma_diag.param_id);
                trace.is_tcc_channel_matching = false; // У QDMA нет аппаратной 1:1 привязки TCC по умолчанию
                trace.mapped_queue = qdma_diag.queue;
                trace.is_param_valid = true;
                return trace;
            }
        }

        return trace;
    }

    int32_t EDMA_Diagnostics::findParamByAddress(const uint32_t address) noexcept
    {
        if (address < REGS::EDMA::PARAM_BASE || address >= (REGS::EDMA::PARAM_BASE + 0x2000))
        {
            return -1;
        }
        return static_cast<int32_t>((address - REGS::EDMA::PARAM_BASE) / 0x20);
    }

    bool EDMA_Diagnostics::diagnoseChannel(const EDMA_DiagnosticSnapshot& s, uint32_t channel, bool is_qdma, EDMA_Channel_Diagnostic& out_diag) noexcept
    {
        if (!is_qdma && channel >= REGS::EDMA::AM335X_DMACH_MAX) return false;
        if (is_qdma && channel >= REGS::EDMA::AM335X_QDMACH_MAX) return false;

        out_diag = is_qdma ? s.qdma[channel] : s.dma[channel];

        return true;
    }

    bool EDMA_Diagnostics::diagnoseTransfer(const EDMA_DiagnosticSnapshot& s, uint32_t tcc, EDMA_TCC_Path_Trace& out_trace) noexcept
    {
        out_trace = findChannelByTCC(s, tcc);
        return out_trace.is_param_valid;
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
