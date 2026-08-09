#ifndef HAL_EDMA_DIAGNOSTICS_2_HPP
#define HAL_EDMA_DIAGNOSTICS_2_HPP


#include "regs/EDMA.hpp"
#include "hal/EDMA/EDMA.hpp"
#include <cstdio>
#include "rtt/rtt_log.h"

#define TAG "edma_diag"

namespace HAL::EDMA
{
    struct EDMAStatusSnapshot
    {
        // CC Status
        uint32_t cc_config;
        uint32_t cc_stat;
        uint32_t cc_err;
        uint32_t emr;
        uint32_t emrh;
        uint32_t qemr;

        // Queue Status
        uint32_t qstat[3];
        uint32_t qwmthra;

        // Shadow Region 0
        uint32_t er, erh;
        uint32_t eer, eerh;
        uint32_t ipr, iprh;
        uint32_t ier, ierh;
        uint32_t ser, serh;
        uint32_t cer, cerh;
        uint32_t qer, qeer, qser;

        // Event Queue Entries (первые 16 из Q0 и Q1 для диагностики)
        uint32_t q0_entries[16];
        uint32_t q1_entries[16];
        uint32_t q2_entries[16];

        // Channel Mappings
        uint32_t dch_map[64];
        uint32_t qch_map[8];
        uint32_t dma_qnum[8];
        uint32_t qdma_qnum;

        // TC0, TC1, TC2 Status
        struct TCStatus
        {
            uint32_t tcstat;
            uint32_t errstat;
            uint32_t errdet;
            uint32_t saopt;
            uint32_t sasrc;
            uint32_t sacnt;
            uint32_t sadst;

            // Destination FIFO status for 4 channels
            uint32_t dfopt[4];
            uint32_t dfsrc[4];
            uint32_t dfcnt[4];
            uint32_t dfdst[4];
        } tc[3];

        // Selected PaRAM entries (можно выборочно)
        REGS::EDMA::paRAM_entry_t param_sets[64];

        // Memory Protection
        uint32_t mpfar;
        uint32_t mpfsr;
        uint32_t mppag;
        uint32_t mppa[8];
    };

    enum class EDMAErrorCategory
    {
        None,
        EventMissed,
        QueueThresholdExceeded,
        TCCError,
        TCError,
        MemoryProtectionError,
        TransferConfigurationError,
        SyncError,
        FIFOError
    };

    struct EDMAErrorInfo
    {
        EDMAErrorCategory category;
        uint32_t channel_or_queue;
        uint32_t error_code;
        char description[128];
    };

    class Diagnostics
    {
    private:
        static constexpr uint32_t AM335X_EDMA3CC_BASE = 0x49000000;

        static REGS::EDMA::AM335X_EDMA3CC_Type* getCC() noexcept
        {
            return reinterpret_cast<REGS::EDMA::AM335X_EDMA3CC_Type*>(AM335X_EDMA3CC_BASE);
        }

        static AM335X_EDMA3TC_Type* getTC(uint32_t tc_num) noexcept
        {
            static const uint32_t tc_bases[] = { 0x49800000, 0x49900000, 0x49A00000 };
            if (tc_num < 3) {
                return reinterpret_cast<REGS::EDMA::AM335X_EDMA3TC_Type*>(tc_bases[tc_num]);
            }
            return nullptr;
        }

    public:
        static EDMAStatusSnapshot captureSnapshot(REGS::EDMA::e_REGION_ID region_id = REGS::EDMA::REGION_0) noexcept
        {
            EDMAStatusSnapshot snap{};
            auto* cc = REGS::EDMA::AM335X_EDMA3CC;

            if (!cc) return snap;

            // CC Status
            snap.cc_config = cc->CCCFG.reg;
            snap.cc_stat = cc->CCSTAT.reg;
            snap.cc_err = cc->CCERR.reg;
            snap.emr = cc->EMR.reg;
            snap.emrh = cc->EMRH.reg;
            snap.qemr = cc->QEMR.reg;

            // Queue Status
            snap.qstat[0] = cc->QSTAT_0.reg;
            snap.qstat[1] = cc->QSTAT_1.reg;
            snap.qstat[2] = cc->QSTAT_2.reg;
            snap.qwmthra = cc->QWMTHRA.reg;

            // Shadow Region (используем регион 0)
            auto get_shadow = [&]() {
                snap.er = cc->S_ER(region_id).reg;
                snap.erh = cc->S_ERH(region_id).reg;
                snap.eer = cc->S_EER(region_id).reg;
                snap.eerh = cc->S_EERH(region_id).reg;
                snap.ipr = cc->S_IPR(region_id).reg;
                snap.iprh = cc->S_IPRH(region_id).reg;
                snap.ier = cc->S_IER(region_id).reg;
                snap.ierh = cc->S_IERH(region_id).reg;
                snap.ser = cc->S_SER(region_id).reg;
                snap.serh = cc->S_SERH(region_id).reg;
                snap.cer = cc->S_CER(region_id).reg;
                snap.cerh = cc->S_CERH(region_id).reg;
                snap.qer = cc->S_QER(region_id).reg;
                snap.qeer = cc->S_QEER(region_id).reg;
                snap.qser = cc->S_QSER(region_id).reg;
            };
            get_shadow();

            // Event Queue Entries
            for (int i = 0; i < 16; ++i) {
                // Q0: 0x400 + i*4
                snap.q0_entries[i] = reinterpret_cast<const volatile uint32_t*>(AM335X_EDMA3CC_BASE + 0x400)[i];
                // Q1: 0x440 + i*4
                snap.q1_entries[i] = reinterpret_cast<const volatile uint32_t*>(AM335X_EDMA3CC_BASE + 0x440)[i];
                // Q2: 0x480 + i*4
                snap.q2_entries[i] = reinterpret_cast<const volatile uint32_t*>(AM335X_EDMA3CC_BASE + 0x480)[i];
            }

            // Channel Mappings
            for (int i = 0; i < 64; ++i) snap.dch_map[i] = cc->DCHMAP[i].reg;
            for (int i = 0; i < 8; ++i) snap.qch_map[i] = cc->QCHMAP[i].reg;
            for (int i = 0; i < 8; ++i) snap.dma_qnum[i] = cc->DMAQNUM[i].reg;
            snap.qdma_qnum = cc->QDMAQNUM.reg;

            // TC Status
            for (int tc = 0; tc < 3; ++tc)
            {
                const auto* tcp = getTC(tc);
                if (tcp)
                {
                    snap.tc[tc].tcstat = tcp->TCSTAT.reg;
                    snap.tc[tc].errstat = tcp->ERRSTAT.reg;
                    snap.tc[tc].errdet = tcp->ERRDET.reg;
                    snap.tc[tc].saopt = tcp->SAOPT.reg;
                    snap.tc[tc].sasrc = tcp->SASRC.reg;
                    snap.tc[tc].sacnt = tcp->SACNT.reg;
                    snap.tc[tc].sadst = tcp->SADST.reg;

                    for (int df = 0; df < 4; ++df)
                    {
                        snap.tc[tc].dfopt[df] = tcp->OPT(df).reg;
                        snap.tc[tc].dfsrc[df] = tcp->SRC(df).reg;
                        snap.tc[tc].dfcnt[df] = tcp->CNT(df).reg;
                        snap.tc[tc].dfdst[df] = tcp->DST(df).reg;
                    }
                }
            }

            // PaRAM Sets (первые 64)
            for (uint32_t i = 0; i < REGS::EDMA::AM335x_PARAMSETS_MAX; ++i)
            {
                snap.param_sets[i] = cc->paRAM(i);
            }

            // Memory Protection
            snap.mpfar = cc->MPFAR.reg;
            snap.mpfsr = cc->MPFSR.reg;
            snap.mppag = cc->MPPAG.reg;
            for (int i = 0; i < 8; ++i)
            {
                snap.mppa[i] = reinterpret_cast<const volatile uint32_t*>(AM335X_EDMA3CC_BASE + 0x810)[i];
            }

            return snap;
        }

        static void printSnapshot(const EDMAStatusSnapshot& snap, REGS::EDMA::e_REGION_ID region_id = REGS::EDMA::REGION_0) noexcept
        {
            RTT_LOG_I(TAG,"\n========== EDMA DIAGNOSTIC SNAPSHOT ==========\n");
            RTT_LOG_I(TAG,"Region: %d\n\n", region_id);

            RTT_LOG_I(TAG,"=== CC Status ===\n");
            RTT_LOG_I(TAG,"CCCFG:   0x%08X\n", snap.cc_config);
            RTT_LOG_I(TAG,"CCSTAT:  0x%08X (Active: %s)\n", snap.cc_stat,
                   snap.cc_stat & 0x1 ? "YES" : "NO");
            RTT_LOG_I(TAG,"CCERR:   0x%08X\n", snap.cc_err);
            RTT_LOG_I(TAG,"EMR:     0x%08X\n", snap.emr);
            RTT_LOG_I(TAG,"EMRH:    0x%08X\n", snap.emrh);
            RTT_LOG_I(TAG,"QEMR:    0x%08X\n", snap.qemr);

            RTT_LOG_I(TAG,"\n=== Event Queue Status ===\n");
            RTT_LOG_I(TAG,"QSTAT0: 0x%08X (Entries: %d)\n", snap.qstat[0], snap.qstat[0] & 0x3F);
            RTT_LOG_I(TAG,"QSTAT1: 0x%08X (Entries: %d)\n", snap.qstat[1], snap.qstat[1] & 0x3F);
            RTT_LOG_I(TAG,"QSTAT2: 0x%08X (Entries: %d)\n", snap.qstat[2], snap.qstat[2] & 0x3F);
            RTT_LOG_I(TAG,"QWMTHRA: 0x%08X\n", snap.qwmthra);

            RTT_LOG_I(TAG,"\n=== Shadow Region (%d) ===\n", region_id);
            RTT_LOG_I(TAG,"ER:      0x%08X (Events pending)\n", snap.er);
            RTT_LOG_I(TAG,"ERH:     0x%08X\n", snap.erh);
            RTT_LOG_I(TAG,"EER:     0x%08X (Events enabled)\n", snap.eer);
            RTT_LOG_I(TAG,"EERH:    0x%08X\n", snap.eerh);
            RTT_LOG_I(TAG,"IPR:     0x%08X (Interrupts pending)\n", snap.ipr);
            RTT_LOG_I(TAG,"IPRH:    0x%08X\n", snap.iprh);
            RTT_LOG_I(TAG,"IER:     0x%08X (Interrupts enabled)\n", snap.ier);
            RTT_LOG_I(TAG,"IERH:    0x%%08X\n", snap.ierh);
            RTT_LOG_I(TAG,"SER:     0x%08X (Secondary events)\n", snap.ser);
            RTT_LOG_I(TAG,"SERH:    0x%08X\n", snap.serh);

            RTT_LOG_I(TAG,"\n=== TC Status ===\n");
            for (int i = 0; i < 3; ++i) {
                RTT_LOG_I(TAG,"TC%d:\n", i);
                RTT_LOG_I(TAG,"  TCSTAT:  0x%08X\n", snap.tc[i].tcstat);
                RTT_LOG_I(TAG,"  ERRSTAT: 0x%08X\n", snap.tc[i].errstat);
                RTT_LOG_I(TAG,"  ERRDET:  0x%08X\n", snap.tc[i].errdet);
                RTT_LOG_I(TAG,"  SAOPT:   0x%08X\n", snap.tc[i].saopt);
                RTT_LOG_I(TAG,"  SASRC:   0x%08X\n", snap.tc[i].sasrc);
                RTT_LOG_I(TAG,"  SACNT:   0x%08X\n", snap.tc[i].sacnt);
                RTT_LOG_I(TAG,"  SADST:   0x%08X\n", snap.tc[i].sadst);
            }

            RTT_LOG_I(TAG,"\n=== Memory Protection ===\n");
            RTT_LOG_I(TAG,"MPFAR:   0x%08X (Fault Address)\n", snap.mpfar);
            RTT_LOG_I(TAG,"MPFSR:   0x%08X (Fault Status)\n", snap.mpfsr);
            RTT_LOG_I(TAG,"MPPAG:   0x%08X\n", snap.mppag);

            RTT_LOG_I(TAG,"\n=== Event Queue Entries (first 4 each) ===\n");
            RTT_LOG_I(TAG,"Q0: ");
            for (int i = 0; i < 4; ++i) RTT_LOG_I(TAG,"0x%08X ", snap.q0_entries[i]);
            RTT_LOG_I(TAG,"\nQ1: ");
            for (int i = 0; i < 4; ++i) RTT_LOG_I(TAG,"0x%08X ", snap.q1_entries[i]);
            RTT_LOG_I(TAG,"\nQ2: ");
            for (int i = 0; i < 4; ++i) RTT_LOG_I(TAG,"0x%08X ", snap.q2_entries[i]);
            RTT_LOG_I(TAG,"\n");
        }

        static EDMAErrorInfo analyzeErrors(const EDMAStatusSnapshot& snap) noexcept
        {
            EDMAErrorInfo info{};
            info.category = EDMAErrorCategory::None;
            info.channel_or_queue = 0;
            info.error_code = 0;
            info.description[0] = '\0';

            // 1. Проверяем EMR/EMRH - пропущенные события DMA
            if (snap.emr != 0) {
                info.category = EDMAErrorCategory::EventMissed;
                info.channel_or_queue = __builtin_ctz(snap.emr);
                info.error_code = snap.emr;
                snprintf(info.description, sizeof(info.description),
                        "DMA Event Missed on channel %d (EMR=0x%08X)",
                        info.channel_or_queue, snap.emr);
                return info;
            }
            if (snap.emrh != 0) {
                info.category = EDMAErrorCategory::EventMissed;
                info.channel_or_queue = 32 + __builtin_ctz(snap.emrh);
                info.error_code = snap.emrh;
                snprintf(info.description, sizeof(info.description),
                        "DMA Event Missed on channel %d (EMRH=0x%08X)",
                        info.channel_or_queue, snap.emrh);
                return info;
            }

            // 2. Проверяем QEMR - пропущенные события QDMA
            if (snap.qemr != 0) {
                info.category = EDMAErrorCategory::EventMissed;
                info.channel_or_queue = __builtin_ctz(snap.qemr);
                info.error_code = snap.qemr;
                snprintf(info.description, sizeof(info.description),
                        "QDMA Event Missed on channel %d (QEMR=0x%08X)",
                        info.channel_or_queue, snap.qemr);
                return info;
            }

            // 3. Проверяем CCERR
            if (snap.cc_err != 0) {
                // TCC Error (бит 16)
                if (snap.cc_err & 0x00010000) {
                    info.category = EDMAErrorCategory::TCCError;
                    info.error_code = snap.cc_err;
                    snprintf(info.description, sizeof(info.description),
                            "TCC Error (CCERR=0x%08X)", snap.cc_err);
                    return info;
                }

                // Queue Threshold Errors (биты 0,1,2)
                if (snap.cc_err & 0x7) {
                    info.category = EDMAErrorCategory::QueueThresholdExceeded;
                    info.channel_or_queue = __builtin_ctz(snap.cc_err & 0x7);
                    info.error_code = snap.cc_err;
                    snprintf(info.description, sizeof(info.description),
                            "Queue Threshold exceeded on Q%d (CCERR=0x%08X)",
                            info.channel_or_queue, snap.cc_err);
                    return info;
                }
            }

            // 4. Проверяем TC ошибки
            for (int i = 0; i < 3; ++i) {
                if (snap.tc[i].errstat != 0) {
                    info.category = EDMAErrorCategory::TCError;
                    info.channel_or_queue = i;
                    info.error_code = snap.tc[i].errstat;
                    uint32_t det = snap.tc[i].errdet;
                    snprintf(info.description, sizeof(info.description),
                            "TC%d Error: ERRSTAT=0x%08X, ERRDET=0x%08X",
                            i, snap.tc[i].errstat, det);
                    return info;
                }
            }

            // 5. Проверяем Memory Protection
            if (snap.mpfsr != 0) {
                info.category = EDMAErrorCategory::MemoryProtectionError;
                info.channel_or_queue = (snap.mpfsr >> 8) & 0x3F; // Channel/TCC
                info.error_code = snap.mpfsr;
                snprintf(info.description, sizeof(info.description),
                        "MP Fault: MPFSR=0x%08X, MPFAR=0x%08X",
                        snap.mpfsr, snap.mpfar);
                return info;
            }

            // 6. Проверяем Source/Destination FIFO ошибки (через TC)
            for (int tc = 0; tc < 3; ++tc) {
                if (snap.tc[tc].saopt & 0x80000000) { // Некорректный OPT
                    info.category = EDMAErrorCategory::TransferConfigurationError;
                    info.channel_or_queue = tc;
                    info.error_code = snap.tc[tc].saopt;
                    snprintf(info.description, sizeof(info.description),
                            "TC%d: Invalid OPT in SAOPT (0x%08X)",
                            tc, snap.tc[tc].saopt);
                    return info;
                }
            }

            // Все хорошо
            snprintf(info.description, sizeof(info.description), "No errors detected");
            return info;
        }

        // Проверка конкретного канала
        static bool diagnoseChannel(uint32_t channel, e_REGION_ID region_id = REGION_0) noexcept
        {
            auto* cc = getCC();
            if (!cc || channel >= 64) return false;

            RTT_LOG_I(TAG,"\n=== Diagnosing Channel %d ===\n", channel);

            // Проверяем что канал включен
            uint32_t mask = (channel < 32) ? (1u << channel) : (1u << (channel - 32));
            bool enabled = false;
            if (channel < 32) {
                enabled = (cc->S_EER(region_id).reg & mask) != 0;
            } else {
                enabled = (cc->S_EERH(region_id).reg & mask) != 0;
            }
            RTT_LOG_I(TAG,"Channel enabled: %s\n", enabled ? "YES" : "NO");

            // Проверяем прерывания
            bool intr_enabled = false;
            if (channel < 32) {
                intr_enabled = (cc->S_IER(region_id).reg & mask) != 0;
            } else {
                intr_enabled = (cc->S_IERH(region_id).reg & mask) != 0;
            }
            RTT_LOG_I(TAG,"Interrupt enabled: %s\n", intr_enabled ? "YES" : "NO");

            // Проверяем наличие события
            bool event_pending = false;
            if (channel < 32) {
                event_pending = (cc->S_ER(region_id).reg & mask) != 0;
            } else {
                event_pending = (cc->S_ERH(region_id).reg & mask) != 0;
            }
            RTT_LOG_I(TAG,"Event pending: %s\n", event_pending ? "YES" : "NO");

            // Проверяем TCC в OPT
            auto param = cc->paRAM(channel);
            RTT_LOG_I(TAG,"PaRAM set for channel %d:\n", channel);
            RTT_LOG_I(TAG,"  OPT:   0x%08X (TCC=%d, TCINTEN=%d)\n",
                   param.OPT.reg, param.OPT.b.TCC, param.OPT.b.TCINTEN);
            RTT_LOG_I(TAG,"  SRC:   0x%08X\n", param.SRC);
            RTT_LOG_I(TAG,"  DST:   0x%08X\n", param.DST);
            RTT_LOG_I(TAG,"  ACNT:  %d\n", param.ACNT);
            RTT_LOG_I(TAG,"  BCNT:  %d\n", param.BCNT);
            RTT_LOG_I(TAG,"  CCNT:  %d\n", param.CCNT);
            RTT_LOG_I(TAG,"  LINK:  0x%04X\n", param.LINK);
            RTT_LOG_I(TAG,"  SRCBIDX: %d\n", param.SRCBIDX);
            RTT_LOG_I(TAG,"  DSTBIDX: %d\n", param.DSTBIDX);
            RTT_LOG_I(TAG,"  SRCCIDX: %d\n", param.SRCCIDX);
            RTT_LOG_I(TAG,"  DSTCIDX: %d\n", param.DSTCIDX);

            // Проверяем наличие ошибок для этого канала
            if (channel < 32) {
                if (cc->EMR.reg & mask) {
                    RTT_LOG_I(TAG,"WARNING: Event Miss detected for this channel!\n");
                }
            } else if (channel >= 32) {
                if (cc->EMRH.reg & (1u << (channel - 32))) {
                    RTT_LOG_I(TAG,"WARNING: Event Miss detected for this channel!\n");
                }
            }

            return true;
        }

        // Проверка регистров TC для конкретного канала
        static void diagnoseTCForChannel(uint32_t channel) noexcept
        {
            auto* cc = getCC();
            if (!cc) return;

            // Определяем номер TC из DMAQNUM
            uint32_t queue = (cc->DMAQNUM[channel >> 3].reg >> ((channel % 8) * 4)) & 0x7;
            RTT_LOG_I(TAG,"Channel %d mapped to Event Queue %d\n", channel, queue);

            // Проверяем статус TC
            if (queue < 3)
            {
                auto* tc = getTC(queue);
                if (tc) {
                    RTT_LOG_I(TAG,"TC%d Status:\n", queue);
                    RTT_LOG_I(TAG,"  TCSTAT: 0x%08X (Current TR: %d)\n", tc->TCSTAT.reg, (tc->TCSTAT.reg >> 8) & 0x3F);
                    RTT_LOG_I(TAG,"  ERRSTAT: 0x%08X\n", tc->ERRSTAT.reg);
                    RTT_LOG_I(TAG,"  ERRDET: 0x%08X\n", tc->ERRDET.reg);
                }
            }
        }
    };
}

#endif //HAL_EDMA_DIAGNOSTICS_2_HPP