#include "EDMA_Diagnostics.hpp"
#include <cstdio>
#include <cstring>

namespace HAL::EDMA
{
namespace
{
    constexpr uintptr_t CC_BASE = 0x49000000u;
    constexpr uintptr_t TC_BASE[3] = {0x49800000u, 0x49900000u, 0x49A00000u};

    inline uint32_t r32(uintptr_t address) noexcept
    {
        return *reinterpret_cast<volatile const uint32_t*>(address);
    }

    inline uint32_t ccreg(uint32_t off) noexcept { return r32(CC_BASE + off); }
    inline uint32_t tcreg(uint32_t tc, uint32_t off) noexcept { return r32(TC_BASE[tc] + off); }

    constexpr uint32_t OPT_SAM       = 1u << 0;
    constexpr uint32_t OPT_DAM       = 1u << 1;
    constexpr uint32_t OPT_SYNCDIM   = 1u << 2;
    constexpr uint32_t OPT_STATIC    = 1u << 3;
    constexpr uint32_t OPT_FWID      = 0x7u << 8;
    constexpr uint32_t OPT_TCCMODE   = 1u << 11;
    constexpr uint32_t OPT_TCC       = 0x3Fu << 12;
    constexpr uint32_t OPT_TCINTEN   = 1u << 20;
    constexpr uint32_t OPT_ITCINTEN  = 1u << 21;
    constexpr uint32_t OPT_TCCHEN    = 1u << 22;
    constexpr uint32_t OPT_ITCCHEN   = 1u << 23;

    constexpr uint32_t TCSTAT_PROGBUSY = 1u << 0;
    constexpr uint32_t TCSTAT_SRCACTV  = 1u << 2;
    constexpr uint32_t TCSTAT_WSACTV   = 1u << 3;
    constexpr uint32_t TCSTAT_DSTACTV  = 0x7u << 4;
    constexpr uint32_t TCSTAT_DFSTRTPTR = 0x3u << 11;

    constexpr uint32_t ERRSTAT_BUSERR  = 1u << 0;
    constexpr uint32_t ERRSTAT_TRERR   = 1u << 2;
    constexpr uint32_t ERRSTAT_MMRAERR = 1u << 3;

    constexpr uint32_t ERRDET_STAT = 0xFu;
    constexpr uint32_t ERRDET_TCC  = 0x3Fu << 8;
    constexpr uint32_t ERRDET_TCINTEN = 1u << 16;
    constexpr uint32_t ERRDET_TCCHEN  = 1u << 17;

    // Shadow region stride is 0x200 bytes on the EDMA3 CC register map.
    constexpr uint32_t SHADOW_BASE = 0x1000u;
    constexpr uint32_t SHADOW_STRIDE = 0x200u;

    void fillRegion(EDMA_CC_Diagnostic::Region& r, uint32_t n) noexcept
    {
        const uint32_t b = SHADOW_BASE + n * SHADOW_STRIDE;
        r.er   = ccreg(b + 0x00); r.erh  = ccreg(b + 0x04);
        r.esr  = ccreg(b + 0x10); r.esrh = ccreg(b + 0x14);
        r.cer  = ccreg(b + 0x18); r.cerh = ccreg(b + 0x1C);
        r.eer  = ccreg(b + 0x20); r.eerh = ccreg(b + 0x24);
        r.ser  = ccreg(b + 0x38); r.serh = ccreg(b + 0x3C);
        r.ier  = ccreg(b + 0x50); r.ierh = ccreg(b + 0x54);
        r.ipr  = ccreg(b + 0x68); r.iprh = ccreg(b + 0x6C);
        r.qer  = ccreg(b + 0x80); r.qeer = ccreg(b + 0x84); r.qser = ccreg(b + 0x90);
    }

    void fillParam(uint32_t id, REGS::EDMA::paRAM_entry_t& p) noexcept
    {
        // Reading the PaRAM as eight words is intentional: it preserves exactly what
        // the hardware contains, including reserved/padding semantics of the user's type.
        const uintptr_t a = CC_BASE + 0x4000u + id * 0x20u;
        auto* dst = reinterpret_cast<uint32_t*>(&p);
        for (uint32_t i = 0; i < 8; ++i) dst[i] = r32(a + i * 4u);
    }

    uint32_t dmaParamId(uint32_t ch) noexcept { return ch; }
    uint32_t qdmaParamId(uint32_t qch) noexcept
    {
        // QCHMAP PAENTRY is bits 5..13. Do not assume the default 32+qch mapping.
        return (ccreg(0x200u + qch * 4u) >> 5) & 0x1FFu;
    }

    EDMA_Channel_Diagnostic captureDMAChannel(uint32_t ch, uint32_t region) noexcept
    {
        EDMA_Channel_Diagnostic d{};
        d.channel = ch;
        d.qdma = false;
        d.dchmap_or_qchmap = ccreg(0x100u + ch * 4u);
        d.param_id = dmaParamId(ch);
        d.param_opt = ccreg(0x4000u + d.param_id * 0x20u);
        d.tcc = (d.param_opt >> 12) & 0x3Fu;
        d.queue = (ccreg(0x240u + (ch / 8u) * 4u) >> ((ch % 8u) * 4u)) & 0x7u;
        fillParam(d.param_id, d.param);

        const uint32_t sb = SHADOW_BASE + region * SHADOW_STRIDE;
        const uint32_t bit = 1u << (ch & 31u);
        const bool hi = ch >= 32;
        const uint32_t ev = ccreg(sb + (hi ? 0x04 : 0x00));
        const uint32_t eer = ccreg(sb + (hi ? 0x24 : 0x20));
        const uint32_t ser = ccreg(sb + (hi ? 0x3C : 0x38));
        const uint32_t ipr = ccreg(sb + (hi ? 0x6C : 0x68));
        const uint32_t ier = ccreg(sb + (hi ? 0x54 : 0x50));
        const uint32_t cer = ccreg(sb + (hi ? 0x1C : 0x18));
        const uint32_t access = ccreg(0x340u + region * 8u + (hi ? 4u : 0u));
        d.event = (ev & bit) != 0;
        d.event_enable = (eer & bit) != 0;
        d.secondary_event = (ser & bit) != 0;
        d.interrupt_pending = (ipr & bit) != 0;
        d.interrupt_enable = (ier & bit) != 0;
        d.chained_event = (cer & bit) != 0;
        d.shadow_access = (access & bit) != 0;
        return d;
    }

    EDMA_Channel_Diagnostic captureQDMAChannel(uint32_t ch, uint32_t region) noexcept
    {
        EDMA_Channel_Diagnostic d{};
        d.channel = ch;
        d.qdma = true;
        d.dchmap_or_qchmap = ccreg(0x200u + ch * 4u);
        d.param_id = qdmaParamId(ch);
        d.param_opt = ccreg(0x4000u + d.param_id * 0x20u);
        d.tcc = (d.param_opt >> 12) & 0x3Fu;
        d.queue = (ccreg(0x260u) >> (ch * 4u)) & 0x7u;
        fillParam(d.param_id, d.param);

        const uint32_t sb = SHADOW_BASE + region * SHADOW_STRIDE;
        const uint32_t bit = 1u << ch;
        d.event = (ccreg(sb + 0x80u) & bit) != 0;
        d.event_enable = (ccreg(sb + 0x84u) & bit) != 0;
        d.secondary_event = (ccreg(sb + 0x90u) & bit) != 0;
        d.shadow_access = (ccreg(0x380u + region * 4u) & bit) != 0;
        return d;
    }

    const char* tcstatState(uint32_t n) noexcept
    {
        switch (n) {
            case 0: return "empty/idle";
            case 1: return "1 TR";
            case 2: return "2 TR";
            case 3: return "3 TR";
            case 4: return "4 TR / FIFO full";
            default: return "reserved/invalid for configured FIFO depth";
        }
    }
}

EDMA_TC_Diagnostic EDMA_Diagnostics::captureTC(uint32_t tc) noexcept
{
    EDMA_TC_Diagnostic d{};
    if (tc >= TCS) return d;
    d.base = static_cast<uint32_t>(TC_BASE[tc]);
    d.pid = tcreg(tc, 0x00); d.tccfg = tcreg(tc, 0x04); d.sysconfig = tcreg(tc, 0x10);
    d.tcstat = tcreg(tc, 0x100); d.errstat = tcreg(tc, 0x120); d.erren = tcreg(tc, 0x124);
    d.errdet = tcreg(tc, 0x12C); d.rdrate = tcreg(tc, 0x140);

    d.saopt = tcreg(tc, 0x240); d.sasrc = tcreg(tc, 0x244); d.sacnt = tcreg(tc, 0x248);
    d.sadst = tcreg(tc, 0x24C); d.sabidx = tcreg(tc, 0x250); d.sampprxy = tcreg(tc, 0x254);
    d.sacntrld = tcreg(tc, 0x258); d.sasrcbref = tcreg(tc, 0x25C); d.sadstbref = tcreg(tc, 0x260);

    d.dfcntrld = tcreg(tc, 0x280); d.dfsrcbref = tcreg(tc, 0x284); d.dfdstbref = tcreg(tc, 0x288);
    for (uint32_t i = 0; i < 4; ++i) {
        const uint32_t b = 0x300u + i * 0x40u;
        d.dfopt[i] = tcreg(tc, b + 0x00); d.dfsrc[i] = tcreg(tc, b + 0x04);
        d.dfcnt[i] = tcreg(tc, b + 0x08); d.dfdst[i] = tcreg(tc, b + 0x0C);
        d.dfbidx[i] = tcreg(tc, b + 0x10); d.dfmppRxy[i] = tcreg(tc, b + 0x14);
    }

    d.tcstat_dfstrtptr = (d.tcstat & TCSTAT_DFSTRTPTR) >> 11;
    d.tcstat_dstactv = (d.tcstat & TCSTAT_DSTACTV) >> 4;
    d.tcstat_wsactive = (d.tcstat & TCSTAT_WSACTV) != 0;
    d.tcstat_srcactive = (d.tcstat & TCSTAT_SRCACTV) != 0;
    d.tcstat_progbusy = (d.tcstat & TCSTAT_PROGBUSY) != 0;

    d.err_bus = (d.errstat & ERRSTAT_BUSERR) != 0;
    d.err_tr = (d.errstat & ERRSTAT_TRERR) != 0;
    d.err_mmr = (d.errstat & ERRSTAT_MMRAERR) != 0;
    d.err_stat = d.errdet & ERRDET_STAT;
    d.err_is_read = d.err_stat >= 1u && d.err_stat <= 7u;
    d.err_is_write = d.err_stat >= 8u;
    d.err_tcc = (d.errdet & ERRDET_TCC) >> 8;
    d.err_tcinten = (d.errdet & ERRDET_TCINTEN) != 0;
    d.err_tcchen = (d.errdet & ERRDET_TCCHEN) != 0;
    d.active_tcc = (d.saopt & OPT_TCC) >> 12;
    d.active_tcinten = (d.saopt & OPT_TCINTEN) != 0;
    d.active_tcchen = (d.saopt & OPT_TCCHEN) != 0;
    return d;
}

EDMA_CC_Diagnostic EDMA_Diagnostics::captureCC() noexcept
{
    EDMA_CC_Diagnostic d{};
    d.base = static_cast<uint32_t>(CC_BASE);
    d.pid = ccreg(0x00); d.cccfg = ccreg(0x04); d.sysconfig = ccreg(0x10);
    for (uint32_t i = 0; i < 64; ++i) d.dchmap[i] = ccreg(0x100u + i * 4u);
    for (uint32_t i = 0; i < 8; ++i) d.qchmap[i] = ccreg(0x200u + i * 4u);
    for (uint32_t i = 0; i < 8; ++i) d.dma_qnum[i] = ccreg(0x240u + i * 4u);
    d.qdma_qnum = ccreg(0x260); d.quepri = ccreg(0x284);
    d.emr = ccreg(0x300); d.emrh = ccreg(0x304); d.qemr = ccreg(0x310);
    d.ccerr = ccreg(0x318); d.eeval = ccreg(0x320);
    for (uint32_t i = 0; i < 8; ++i) {
        d.drae[i] = ccreg(0x340u + i * 8u);
        d.draeh[i] = ccreg(0x344u + i * 8u);
        d.qrae[i] = ccreg(0x380u + i * 4u);
        d.mppa[i] = ccreg(0x810u + i * 4u);
        fillRegion(d.region[i], i);
    }
    for (uint32_t q = 0; q < 3; ++q) d.qstat[q] = ccreg(0x600u + q * 4u);
    d.qwmthra = ccreg(0x620); d.ccstat = ccreg(0x640);
    d.mpfar = ccreg(0x800); d.mpfsr = ccreg(0x804); d.mppag = ccreg(0x80C);
    for (uint32_t q = 0; q < 3; ++q)
        for (uint32_t e = 0; e < 16; ++e)
            d.qentry[q][e] = ccreg(0x400u + q * 0x40u + e * 4u);
    return d;
}

EDMA_DiagnosticSnapshot EDMA_Diagnostics::capture() noexcept
{
    EDMA_DiagnosticSnapshot s{};
    s.region_id = static_cast<uint32_t>(get_region_id());
    if (s.region_id >= REGIONS) s.region_id = 0;
    s.cc = captureCC();
    for (uint32_t i = 0; i < TCS; ++i) s.tc[i] = captureTC(i);
    for (uint32_t i = 0; i < DMA_CHANNELS; ++i) s.dma[i] = captureDMAChannel(i, s.region_id);
    for (uint32_t i = 0; i < QDMA_CHANNELS; ++i) s.qdma[i] = captureQDMAChannel(i, s.region_id);
    return s;
}

std::string EDMA_Diagnostics::tcErrorSummary(const EDMA_TC_Diagnostic& d)
{
    char b[768];
    const char* stat = d.err_stat == 0 ? "no transaction error" :
                       (d.err_is_read ? "read error" : "write error");
    std::snprintf(b, sizeof(b),
        "ERRSTAT=%08X [BUSERR=%u TRERR=%u MMRAERR=%u], ERRDET=%08X [STAT=%X (%s), TCC=%u, TCINTEN=%u, TCCHEN=%u]; "
        "TCSTAT=%08X [PROGBUSY=%u SRCACTV=%u WSACTV=%u DSTACTV=%u DFSTRTPTR=%u]; "
        "ACTIVE: SRC=%08X DST=%08X SACNT=%08X TCC=%u TCINTEN=%u TCCHEN=%u",
        d.errstat, d.err_bus, d.err_tr, d.err_mmr, d.errdet, d.err_stat, stat,
        d.err_tcc, d.err_tcinten, d.err_tcchen, d.tcstat,
        d.tcstat_progbusy, d.tcstat_srcactive, d.tcstat_wsactive, d.tcstat_dstactv,
        d.tcstat_dfstrtptr, d.sasrc, d.sadst, d.sacnt, d.active_tcc,
        d.active_tcinten, d.active_tcchen);
    return b;
}

std::string EDMA_Diagnostics::ccErrorSummary(const EDMA_CC_Diagnostic& d)
{
    char b[512];
    std::snprintf(b, sizeof(b),
        "EMR=%08X EMRH=%08X QEMR=%08X CCERR=%08X EEVAL=%08X; "
        "QSTAT={%08X,%08X,%08X} CCSTAT=%08X MPFAR=%08X MPFSR=%08X",
        d.emr, d.emrh, d.qemr, d.ccerr, d.eeval,
        d.qstat[0], d.qstat[1], d.qstat[2], d.ccstat, d.mpfar, d.mpfsr);
    return b;
}

std::string EDMA_Diagnostics::channelSummary(const EDMA_Channel_Diagnostic& d)
{
    char b[512];
    std::snprintf(b, sizeof(b),
        "%s CH=%u PaRAM=%u TCC=%u Q=%u OPT=%08X [EV=%u EER=%u SER=%u IPR=%u IER=%u CER=%u ACCESS=%u]",
        d.qdma ? "QDMA" : "DMA", d.channel, d.param_id, d.tcc, d.queue, d.param_opt,
        d.event, d.event_enable, d.secondary_event, d.interrupt_pending,
        d.interrupt_enable, d.chained_event, d.shadow_access);
    return b;
}

std::string EDMA_Diagnostics::toText(const EDMA_DiagnosticSnapshot& s)
{
    std::string out;
    out.reserve(24000);
    out += "=== AM335x EDMA3 DIAGNOSTIC SNAPSHOT ===\n";
    out += "region=" + std::to_string(s.region_id) + "\n";
    out += "CC: " + ccErrorSummary(s.cc) + "\n";
    for (uint32_t i = 0; i < TCS; ++i) {
        out += "TC" + std::to_string(i) + ": " + tcErrorSummary(s.tc[i]) + "\n";
        out += "  TCCFG=" + std::to_string(s.tc[i].tccfg) + " RDRATE=" + std::to_string(s.tc[i].rdrate) + "\n";
        out += "  DFSTRTPTR=" + std::to_string(s.tc[i].tcstat_dfstrtptr) +
               " DSTACTV=" + std::to_string(s.tc[i].tcstat_dstactv) +
               " state=" + tcstatState(s.tc[i].tcstat_dstactv) + "\n";
    }
    for (const auto& d : s.dma) {
        if (d.event || d.event_enable || d.secondary_event || d.interrupt_pending || d.interrupt_enable || d.chained_event)
            out += channelSummary(d) + "\n";
    }
    for (const auto& d : s.qdma) {
        if (d.event || d.event_enable || d.secondary_event)
            out += channelSummary(d) + "\n";
    }
    return out;
}

void EDMA_Diagnostics::clearTCError(uint32_t tc, uint32_t mask) noexcept
{
    if (tc < TCS) *reinterpret_cast<volatile uint32_t*>(TC_BASE[tc] + 0x128u) = mask;
}

void EDMA_Diagnostics::clearCCErrors(uint32_t mask) noexcept
{
    *reinterpret_cast<volatile uint32_t*>(CC_BASE + 0x31Cu) = mask;
}
}
