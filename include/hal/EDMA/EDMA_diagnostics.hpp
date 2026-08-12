#ifndef HAL_EDMA_DIAGNOSTICS_HPP
#define HAL_EDMA_DIAGNOSTICS_HPP

#include <array>
#include <cstdint>
#include <string_view>
#include <cstddef>
#include "hal/EDMA/EDMA.hpp"

namespace HAL::EDMA
{
    /**
     * @brief Ошибки модуля Transfer Controller (TC) в соответствии со структурой TI:
     * ERRSTAT
     *   ├── BUSERR
     *   ├── TRERR
     *   └── MMRAERR
     *        └── ERRDET (STAT, TCC, TCINTEN, TCCHEN)
     */
    struct EDMA_TC_Error_Details
    {
        bool bus_error{};
        bool tr_error{};
        bool mmra_error{};

        // Поля из регистра ERRDET при MMRAERR
        uint32_t stat{};
        uint32_t tcc{};
        bool tcinten{};
        bool tcchen{};
    };

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

        // Active register set (Source)
        uint32_t saopt{};
        uint32_t sasrc{};
        uint32_t sacnt{};
        uint32_t sadst{};
        uint32_t sabidx{};
        uint32_t sampprxy{};
        uint32_t sacntrld{};
        uint32_t sasrcbref{};
        uint32_t sadstbref{};

        // Destination FIFO (4 набора на AM335x)
        std::array<uint32_t, 4> dfopt{};
        std::array<uint32_t, 4> dfsrc{};
        std::array<uint32_t, 4> dfcnt{};
        std::array<uint32_t, 4> dfdst{};
        std::array<uint32_t, 4> dfbidx{};
        std::array<uint32_t, 4> dfmppRxy{};

        uint32_t dfcntrld{};
        uint32_t dfsrcbref{};
        uint32_t dfdstbref{};

        // Декодированные состояния TCSTAT
        uint32_t tcstat_dfstrtptr{};
        uint32_t tcstat_dstactv{};
        bool tcstat_wsactive{};
        bool tcstat_srcactive{};
        bool tcstat_progbusy{};

        // Декодированные ошибки TC по структуре TI
        EDMA_TC_Error_Details errors{};
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
        std::array<std::array<uint32_t, 16>, 3> qentry{};
    };

    struct EDMA_Channel_Diagnostic
    {
        uint32_t channel{};
        bool is_qdma{};
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

    /**
     * @brief Полный путь трассировки цепочки
     * TCC -> PaRAM -> DMA/QDMA Channel -> Event Queue -> DCHMAP/QCHMAP
     */
    struct EDMA_TCC_Path_Trace
    {
        uint32_t target_tcc{0xFFFFFFFF};
        int32_t found_dma_channel{-1};
        int32_t found_qdma_channel{-1};
        int32_t param_id{-1};
        uint32_t mapped_queue{0xFFFFFFFF};
        bool is_tcc_channel_matching{false};
        bool is_param_valid{false};
    };

    struct EDMA_DiagnosticSnapshot
    {
        uint32_t region_id{};
        EDMA_CC_Diagnostic cc{};
        std::array<EDMA_TC_Diagnostic, 3> tc{};

        std::array<EDMA_Channel_Diagnostic, 64> dma{};
        std::array<EDMA_Channel_Diagnostic, 8> qdma{};
    };

    class EDMA_Diagnostics final
    {
    public:
        static constexpr uint32_t DMA_CHANNELS  = 64;
        static constexpr uint32_t QDMA_CHANNELS = 8;
        static constexpr uint32_t TCS           = 3;
        static constexpr uint32_t REGIONS       = 8;

        // Бездинамическое снятие снимков
        static EDMA_DiagnosticSnapshot capture() noexcept;
        static EDMA_TC_Diagnostic captureTC(uint32_t tc_idx) noexcept;
        static EDMA_CC_Diagnostic captureCC() noexcept;

        // Поисковые методы и диагностика связей (без std::string)
        static EDMA_TCC_Path_Trace findChannelByTCC(const EDMA_DiagnosticSnapshot& s, uint32_t tcc) noexcept;
        static int32_t findParamByAddress(uint32_t address) noexcept;

        static bool diagnoseChannel(const EDMA_DiagnosticSnapshot& s, uint32_t channel, bool is_qdma, EDMA_Channel_Diagnostic& out_diag) noexcept;
        static bool diagnoseTransfer(const EDMA_DiagnosticSnapshot& s, uint32_t tcc, EDMA_TCC_Path_Trace& out_trace) noexcept;

        // Функция форматирования лога напрямую в буфер (для RTT)
        static size_t decodeCC(const EDMA_DiagnosticSnapshot& s, char* buf, size_t max_len) noexcept;
        static size_t decodeTC(const EDMA_DiagnosticSnapshot& s, char* buf, size_t max_len, uint32_t channel, uint8_t tc_idx = 0, bool is_qdma = false) noexcept;
        static size_t decodeChannel(const EDMA_DiagnosticSnapshot& s, char* buf, size_t max_len, uint32_t channel, bool is_qdma = false) noexcept;
        // Очистка аппаратных регистров
        static void clearTCError(uint32_t tc_idx, uint32_t mask) noexcept;
        static void clearCCErrors(uint32_t mask) noexcept;
    private:

        static REGS::EDMA::AM335x_EDMA3TC_Type* tc_arr[REGS::EDMA::AM335x_TCS_MAX];
    };
}

#endif // HAL_EDMA_DIAGNOSTICS_HPP