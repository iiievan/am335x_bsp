#ifndef HAL_EDMA_DIAGNOSTICS_HPP
#define HAL_EDMA_DIAGNOSTICS_HPP

#include "regs/EDMA.hpp"
#include "hal/EDMA/EDMA.hpp"
#include <cstdint>
#include "rtt/rtt_log.h"

#define TAG "edma_diag"

namespace HAL::EDMA
{
    struct TcSnapshot
    {
        uint32_t base;
        uint32_t tcstat;
        uint32_t errstat;
        uint32_t errdet;
        uint32_t erren;

        // Source Active
        uint32_t saopt;
        uint32_t sasrc;
        uint32_t sacnt;
        uint32_t sadst;
        uint32_t sabidx;

        // Destination FIFO sets (0..3)
        struct DfSet {
            uint32_t opt;
            uint32_t src;
            uint32_t cnt;
            uint32_t dst;
            uint32_t bidx;
        } df[4];
    };

    struct CcSnapshot
    {
        // Errors / Misses
        uint32_t emr;
        uint32_t emrh;
        uint32_t qemr;
        uint32_t ccerr;

        // Event path
        uint32_t er, erh;
        uint32_t eer, eerh;
        uint32_t ser, serh;
        uint32_t cer, cerh;
        uint32_t esr, esrh;          // manual set

        // QDMA path
        uint32_t qer;
        uint32_t qeer;
        uint32_t qser;

        // Interrupt path
        uint32_t ier, ierh;
        uint32_t ipr, iprh;

        // Queue / status
        uint32_t qstat[3];
        uint32_t qwmthra;
        uint32_t ccstat;

        // Memory protection
        uint32_t mpfar;
        uint32_t mpfsr;

        // Region
        REGS::EDMA::e_REGION_ID region;
    };

    struct ChannelSnapshot
    {
        uint8_t  ch_num;
        bool     is_qdma;
        uint32_t dchmap;          // only for DMA
        uint32_t qchmap;          // only for QDMA
        uint32_t queue_num;       // из DMAQNUM / QDMAQNUM
        bool     shadow_enabled;  // DRAE / QRAE
        bool     event_enabled;   // EER / QEER
        bool     secondary_set;   // SER / QSER
        bool     miss_set;        // EMR / QEMR
        bool     interrupt_enabled;
        bool     interrupt_pending;
        REGS::EDMA::paRAM_entry_t param;
    };

    class EdmaDiagnostics
    {
    public:

        static void captureFull(CcSnapshot& cc, TcSnapshot tc[3]) noexcept
        {
            captureCc(cc);
            captureTc(0, tc[0]);
            captureTc(1, tc[1]);
            captureTc(2, tc[2]);
        }

        static void captureCc(CcSnapshot& s) noexcept
        {
            using namespace REGS::EDMA;
            const auto& cc = *AM335X_EDMA3CC;
            const auto region = HAL::EDMA::get_region_id();

            s.region = region;

            s.emr   = cc.EMR.reg;
            s.emrh  = cc.EMRH.reg;
            s.qemr  = cc.QEMR.reg;
            s.ccerr = cc.CCERR.reg;

            s.er    = cc.S_ER(region).reg;
            s.erh   = cc.ERH.reg;               // global high (если нужно)
            s.eer   = cc.S_EER(region).reg;
            s.eerh  = cc.S_EERH(region).reg;
            s.ser   = cc.S_SER(region).reg;
            s.serh  = cc.S_SERH(region).reg;
            s.cer   = cc.S_CER(region).reg;
            s.cerh  = cc.CERH.reg;
            s.esr   = cc.S_ESR(region).reg;
            s.esrh  = cc.S_ESRH(region).reg;

            s.qer   = cc.S_QER(region).reg;
            s.qeer  = cc.S_QEER(region).reg;
            s.qser  = cc.S_QSER(region).reg;

            s.ier   = cc.S_IER(region).reg;
            s.ierh  = cc.S_IERH(region).reg;
            s.ipr   = cc.S_IPR(region).reg;
            s.iprh  = cc.S_IPRH(region).reg;

            s.qstat[0] = cc.QSTAT_0.reg;
            s.qstat[1] = cc.QSTAT_1.reg;
            s.qstat[2] = cc.QSTAT_2.reg;
            s.qwmthra  = cc.QWMTHRA.reg;
            s.ccstat   = cc.CCSTAT.reg;

            s.mpfar = cc.MPFAR.reg;
            s.mpfsr = cc.MPFSR.reg;
        }

        static void captureTc(const uint8_t tc_idx, TcSnapshot& s) noexcept
        {
            using namespace REGS::EDMA;
            const AM335x_EDMA3TC_Type* tc = nullptr;

            switch (tc_idx)
            {
                case 0: tc = AM335X_EDMA3TC0; s.base = AM335x_EDMA3TC0_BASE; break;
                case 1: tc = AM335X_EDMA3TC1; s.base = AM335x_EDMA3TC1_BASE; break;
                case 2: tc = AM335X_EDMA3TC2; s.base = AM335x_EDMA3TC2_BASE; break;
                default: return;
            }

            s.tcstat  = tc->TCSTAT.reg;
            s.errstat = tc->ERRSTAT.reg;
            s.errdet  = tc->ERRDET.reg;
            s.erren   = tc->ERREN.reg;

            s.saopt = tc->SAOPT.reg;
            s.sasrc = tc->SASRC.reg;
            s.sacnt = tc->SACNT.reg;
            s.sadst = tc->SADST.reg;
            s.sabidx= tc->SABIDX.reg;

            for (uint32_t i = 0; i < 4; ++i)
            {
                s.df[i].opt  = tc->OPT(i).reg;
                s.df[i].src  = tc->SRC(i).reg;
                s.df[i].cnt  = tc->CNT(i).reg;
                s.df[i].dst  = tc->DST(i).reg;
                s.df[i].bidx = tc->BIDX(i).reg;
            }
        }

        static ChannelSnapshot captureChannel(const uint8_t ch, const bool is_qdma = false) noexcept
        {
            using namespace REGS::EDMA;

            ChannelSnapshot s{};
            s.ch_num  = ch;
            s.is_qdma = is_qdma;

            const auto& cc = *AM335X_EDMA3CC;
            const auto region = HAL::EDMA::get_region_id();

            if (!is_qdma)
            {
                s.dchmap = cc.DCHMAP[ch].reg;
                // queue number
                const uint32_t qreg = cc.DMAQNUM[ch >> 3].reg;
                s.queue_num = (qreg >> ((ch % 8) * 4)) & 0x7;

                if (ch < 32)
                {
                    s.shadow_enabled   = (cc.DRAE(region).reg  & (1u << ch)) != 0;
                    s.event_enabled    = (cc.S_EER(region).reg & (1u << ch)) != 0;
                    s.secondary_set    = (cc.S_SER(region).reg & (1u << ch)) != 0;
                    s.miss_set         = (cc.EMR.reg           & (1u << ch)) != 0;
                    s.interrupt_enabled= (cc.S_IER(region).reg & (1u << ch)) != 0;
                    s.interrupt_pending= (cc.S_IPR(region).reg & (1u << ch)) != 0;
                }
                else
                {
                    const uint32_t bit = ch - 32;
                    s.shadow_enabled   = (cc.DRAEH(region).reg  & (1u << bit)) != 0;
                    s.event_enabled    = (cc.S_EERH(region).reg & (1u << bit)) != 0;
                    s.secondary_set    = (cc.S_SERH(region).reg & (1u << bit)) != 0;
                    s.miss_set         = (cc.EMRH.reg           & (1u << bit)) != 0;
                    s.interrupt_enabled= (cc.S_IERH(region).reg & (1u << bit)) != 0;
                    s.interrupt_pending= (cc.S_IPRH(region).reg & (1u << bit)) != 0;
                }

                const uint32_t param_id = (s.dchmap >> 5) & 0x1FF;
                HAL::EDMA::get_paRAM(param_id, s.param);
            }
            else
            {
                s.qchmap = cc.QCHMAP[ch].reg;
                s.queue_num = (cc.QDMAQNUM.reg >> (ch * 4)) & 0x7;

                s.shadow_enabled    = (cc.QRAE[region].reg & (1u << ch)) != 0;
                s.event_enabled     = (cc.S_QEER(region).reg & (1u << ch)) != 0;
                s.secondary_set     = (cc.S_QSER(region).reg & (1u << ch)) != 0;
                s.miss_set          = (cc.QEMR.reg          & (1u << ch)) != 0;

                const uint32_t param_id = (s.qchmap >> 5) & 0x1FF;
                HAL::EDMA::QDMA_get_paRAM(param_id, s.param);

                // Interrupt on TCC (NOT on qch)
                const uint8_t tcc = s.param.OPT.b.TCC;
                if (tcc < 32)
                {
                    s.interrupt_enabled = (cc.S_IER(region).reg & (1u << tcc)) != 0;
                    s.interrupt_pending = (cc.S_IPR(region).reg & (1u << tcc)) != 0;
                }
                else
                {
                    const uint32_t bit = tcc - 32;
                    s.interrupt_enabled = (cc.S_IERH(region).reg & (1u << bit)) != 0;
                    s.interrupt_pending = (cc.S_IPRH(region).reg & (1u << bit)) != 0;
                }
            }

            return s;
        }

        static void dumpChannel(const ChannelSnapshot& s) noexcept
        {
            RTT_LOG_I(TAG,"=== Channel %u (%s) ===", s.ch_num, s.is_qdma ? "QDMA" : "DMA");
            RTT_LOG_I(TAG,"  Queue          : %u", (unsigned)s.queue_num);
            RTT_LOG_I(TAG,"  Shadow enabled : %s", s.shadow_enabled ? "YES" : "NO");
            RTT_LOG_I(TAG,"  Event enabled  : %s", s.event_enabled  ? "YES" : "NO");
            RTT_LOG_I(TAG,"  Secondary set  : %s", s.secondary_set  ? "YES" : "NO");
            RTT_LOG_I(TAG,"  Missed event   : %s", s.miss_set       ? "YES" : "NO");
            RTT_LOG_I(TAG,"  Intr enabled   : %s", s.interrupt_enabled ? "YES" : "NO");
            RTT_LOG_I(TAG,"  Intr pending   : %s", s.interrupt_pending ? "YES" : "NO");

            if (!s.is_qdma)
                RTT_LOG_I(TAG,"  DCHMAP         : 0x%08X (PaRAM %u)", (unsigned)s.dchmap, (unsigned)((s.dchmap >> 5) & 0x1FF));
            else
                RTT_LOG_I(TAG,"  QCHMAP         : 0x%08X (PaRAM %u, trigword %u)", (unsigned)s.qchmap, (unsigned)((s.qchmap >> 5) & 0x1FF), (unsigned)((s.qchmap >> 2) & 0x7));

            const auto& p = s.param;
            RTT_LOG_I(TAG,"  PaRAM OPT      : 0x%08X", (unsigned)p.OPT.reg);
            RTT_LOG_I(TAG,"    SAM=%u DAM=%u SYNCDIM=%u STATIC=%u FWID=%u", p.OPT.b.SAM, p.OPT.b.DAM, p.OPT.b.SYNCDIM, p.OPT.b.STATIC, p.OPT.b.FWID);
            RTT_LOG_I(TAG,"    TCC=%u TCINTEN=%u ITCINTEN=%u TCCHEN=%u", p.OPT.b.TCC, p.OPT.b.TCINTEN, p.OPT.b.ITCINTEN, p.OPT.b.TCCHEN);
            RTT_LOG_I(TAG,"  SRC=0x%08X  DST=0x%08X", (unsigned)p.SRC, (unsigned)p.DST);
            RTT_LOG_I(TAG,"  ACNT=%u BCNT=%u CCNT=%u", p.ACNT, p.BCNT, p.CCNT);
            RTT_LOG_I(TAG,"  SRCBIDX=%d DSTBIDX=%d  SRCCIDX=%d DSTCIDX=%d", p.SRCBIDX, p.DSTBIDX, p.SRCCIDX, p.DSTCIDX);
            RTT_LOG_I(TAG,"  LINK=0x%04X  BCNTRLD=%u", p.LINK, p.BCNTRLD);
        }

        static void dumpCc(const CcSnapshot& s) noexcept
        {
            RTT_LOG_I(TAG,"=== CC Snapshot (region %u) ===", static_cast<unsigned>(s.region));
            RTT_LOG_I(TAG,"  EMR  = 0x%08X  EMRH = 0x%08X  QEMR = 0x%08X", (unsigned)s.emr, (unsigned)s.emrh, (unsigned)s.qemr);
            RTT_LOG_I(TAG,"  CCERR= 0x%08X", (unsigned)s.ccerr);
            RTT_LOG_I(TAG,"  EER  = 0x%08X  EERH = 0x%08X", (unsigned)s.eer, (unsigned)s.eerh);
            RTT_LOG_I(TAG,"  SER  = 0x%08X  SERH = 0x%08X", (unsigned)s.ser, (unsigned)s.serh);
            RTT_LOG_I(TAG,"  IPR  = 0x%08X  IPRH = 0x%08X", (unsigned)s.ipr, (unsigned)s.iprh);
            RTT_LOG_I(TAG,"  IER  = 0x%08X  IERH = 0x%08X", (unsigned)s.ier, (unsigned)s.ierh);
            RTT_LOG_I(TAG,"  QSTAT0=0x%08X QSTAT1=0x%08X QSTAT2=0x%08X", (unsigned)s.qstat[0], (unsigned)s.qstat[1], (unsigned)s.qstat[2]);
            RTT_LOG_I(TAG,"  CCSTAT=0x%08X  QWMTHRA=0x%08X", (unsigned)s.ccstat, (unsigned)s.qwmthra);
            RTT_LOG_I(TAG,"  MPFAR=0x%08X  MPFSR=0x%08X", (unsigned)s.mpfar, (unsigned)s.mpfsr);
        }

        static void dumpTc(const TcSnapshot& s) noexcept
        {
            RTT_LOG_I(TAG,"=== TC @ 0x%08X ===", (unsigned)s.base);
            RTT_LOG_I(TAG,"  TCSTAT = 0x%08X", (unsigned)s.tcstat);
            RTT_LOG_I(TAG,"  ERRSTAT= 0x%08X  ERRDET=0x%08X  ERREN=0x%08X", (unsigned)s.errstat, (unsigned)s.errdet, (unsigned)s.erren);
            RTT_LOG_I(TAG,"  SAOPT=0x%08X SASRC=0x%08X SACNT=0x%08X SADST=0x%08X", (unsigned)s.saopt, (unsigned)s.sasrc, (unsigned)s.sacnt, (unsigned)s.sadst);

            for (int i = 0; i < 4; ++i)
            {
                if (s.df[i].opt || s.df[i].cnt)
                {
                    RTT_LOG_I(TAG,"  DF[%d] OPT=0x%08X SRC=0x%08X CNT=0x%08X DST=0x%08X", i, (unsigned)s.df[i].opt, (unsigned)s.df[i].src, (unsigned)s.df[i].cnt, (unsigned)s.df[i].dst);
                }
            }
        }

        static void diagnoseSilentChannel(const uint8_t ch, const bool is_qdma = false) noexcept
        {
            const auto chs = captureChannel(ch, is_qdma);
            dumpChannel(chs);

            CcSnapshot cc{};
            captureCc(cc);
            dumpCc(cc);

            if (chs.miss_set)
                RTT_LOG_E(TAG,">>> EVENT MISSED! Clear EMR/QEMR and check why channel was not ready.");

            if (!chs.event_enabled)
                RTT_LOG_E(TAG,">>> Event is NOT enabled (EER/QEER).");

            if (!chs.interrupt_enabled)
                RTT_LOG_E(TAG,">>> Completion interrupt is NOT enabled for TCC.");

            if (chs.param.ACNT == 0)
                RTT_LOG_E(TAG,">>> ACNT == 0 → transfer will never start.");

            if (is_qdma && chs.param.OPT.b.STATIC == 0)
                RTT_LOG_E(TAG,">>> QDMA without STATIC bit — careful with linking.");
        }
    };
} // namespace HAL::EDMA

#endif // HAL_EDMA_DIAGNOSTICS_HPP