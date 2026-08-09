#ifndef HAL_EDMA_DIAGNOSTICS_HPP
#define HAL_EDMA_DIAGNOSTICS_HPP

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include "hal/EDMA/EDMA.hpp"

namespace HAL::EDMA
{
    /**
     * Snapshot-only diagnostic module for AM335x EDMA3.
     *
     * IMPORTANT: capture() is intentionally read-only. It does not clear any
     * error/event/interrupt status, because ERRDET and the active TC state are
     * most useful before the normal ISR clears anything.
     */
    struct EDMA_TC_Diagnostic
    {
        uint32_t base{};
        uint32_t pid{};
        uint32_t tccfg{};
        uint32_t sysconfig{};
        uint32_t tcstat{};
        uint32_t errstat{};
        uint32_t erren{};
        uint32_t errdet{};
        uint32_t rdrate{};

        // Source-active register set.
        uint32_t saopt{};
        uint32_t sasrc{};
        uint32_t sacnt{};
        uint32_t sadst{};
        uint32_t sabidx{};
        uint32_t sampprxy{};
        uint32_t sacntrld{};
        uint32_t sasrcbref{};
        uint32_t sadstbref{};

        // Destination FIFO register sets. AM335x exposes four sets here.
        std::array<uint32_t, 4> dfopt{};
        std::array<uint32_t, 4> dfsrc{};
        std::array<uint32_t, 4> dfcnt{};
        std::array<uint32_t, 4> dfdst{};
        std::array<uint32_t, 4> dfbidx{};
        std::array<uint32_t, 4> dfmppRxy{};

        uint32_t dfcntrld{};
        uint32_t dfsrcbref{};
        uint32_t dfdstbref{};

        // Decoded high-value state.
        uint32_t tcstat_dfstrtptr{};
        uint32_t tcstat_dstactv{};
        bool tcstat_wsactive{};
        bool tcstat_srcactive{};
        bool tcstat_progbusy{};

        bool err_bus{};
        bool err_tr{};
        bool err_mmr{};
        uint32_t err_stat{};
        bool err_is_read{};
        bool err_is_write{};

        uint32_t err_tcc{};
        bool err_tcinten{};
        bool err_tcchen{};

        uint32_t active_tcc{};
        bool active_tcinten{};
        bool active_tcchen{};
    };

    struct EDMA_CC_Diagnostic
    {
        uint32_t base{};
        uint32_t pid{};
        uint32_t cccfg{};
        uint32_t sysconfig{};

        std::array<uint32_t, 64> dchmap{};
        std::array<uint32_t, 8> qchmap{};
        std::array<uint32_t, 8> dma_qnum{};
        uint32_t qdma_qnum{};
        uint32_t quepri{};

        uint32_t emr{};
        uint32_t emrh{};
        uint32_t qemr{};
        uint32_t ccerr{};
        uint32_t eeval{};

        std::array<uint32_t, 8> drae{};
        std::array<uint32_t, 8> draeh{};
        std::array<uint32_t, 8> qrae{};

        std::array<uint32_t, 3> qstat{};
        uint32_t qwmthra{};
        uint32_t ccstat{};

        uint32_t mpfar{};
        uint32_t mpfsr{};
        uint32_t mppag{};
        std::array<uint32_t, 8> mppa{};

        // Shadow-region state. Eight regions are captured because the AM335x
        // register map exposes eight region slots, even if firmware uses only one.
        struct Region
        {
            uint32_t er{}, erh{};
            uint32_t esr{}, esrh{};
            uint32_t cer{}, cerh{};
            uint32_t eer{}, eerh{};
            uint32_t ser{}, serh{};
            uint32_t ier{}, ierh{};
            uint32_t ipr{}, iprh{};
            uint32_t qer{}, qeer{}, qser{};
        };
        std::array<Region, 8> region{};

        // Queue RAM visibility. Three queues are present in the AM335x register map,
        // 16 entries each. The entry value is kept raw because its exact bit meaning
        // is queue/debug-state dependent.
        std::array<std::array<uint32_t, 16>, 3> qentry{};
    };

    struct EDMA_Channel_Diagnostic
    {
        uint32_t channel{};
        bool qdma{};
        uint32_t tcc{};
        uint32_t param_id{};
        uint32_t dchmap_or_qchmap{};
        uint32_t queue{};
        uint32_t param_opt{};
        REGS::EDMA::paRAM_entry_t param{};
        bool event{};
        bool event_enable{};
        bool secondary_event{};
        bool interrupt_pending{};
        bool interrupt_enable{};
        bool chained_event{};
        bool shadow_access{};
    };

    struct EDMA_DiagnosticSnapshot
    {
        uint32_t region_id{};
        EDMA_CC_Diagnostic cc{};
        std::array<EDMA_TC_Diagnostic, 3> tc{};

        // Captured logical channel/PaRAM information. A snapshot of all 64 DMA
        // channels and all 8 QDMA channels makes post-mortem analysis possible even
        // when the failing channel is not known by the caller.
        std::array<EDMA_Channel_Diagnostic, 64> dma{};
        std::array<EDMA_Channel_Diagnostic, 8> qdma{};
    };

    class EDMA_Diagnostics final
    {
    public:
        static constexpr uint32_t DMA_CHANNELS  = 64;
        static constexpr uint32_t QDMA_CHANNELS = 8;
        static constexpr uint32_t TCS = 3;
        static constexpr uint32_t REGIONS = 8;

        static EDMA_DiagnosticSnapshot capture() noexcept;
        static EDMA_TC_Diagnostic captureTC(uint32_t tc) noexcept;
        static EDMA_CC_Diagnostic captureCC() noexcept;

        // Decode without touching hardware. Useful in a debugger/log parser.
        static std::string tcErrorSummary(const EDMA_TC_Diagnostic& tc);
        static std::string ccErrorSummary(const EDMA_CC_Diagnostic& cc);
        static std::string channelSummary(const EDMA_Channel_Diagnostic& ch);

        // Optional human-readable dump. Keep the register snapshot itself as the
        // canonical diagnostic artifact.
        static std::string toText(const EDMA_DiagnosticSnapshot& s);

        // Clear functions are deliberately explicit and separate from capture().
        static void clearTCError(uint32_t tc, uint32_t mask) noexcept;
        static void clearCCErrors(uint32_t mask) noexcept;
    };
}

#endif
