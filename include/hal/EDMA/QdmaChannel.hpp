#ifndef HAL_QDMACHANNEL_HPP
#define HAL_QDMACHANNEL_HPP

#include "regs/EDMA.hpp"
#include "hal/EDMA/EDMA.hpp"
#include "InterruptDispatcher.hpp"

namespace HAL::EDMA
{
    // Индексы слов PaRAM для настройки Trigger Word (0 = OPT, 1 = SRC, ..., 7 = CCNT)
    enum class QdmaTrigWord : uint8_t
    {
        OPT   = 0,
        SRC   = 1,
        ACNT_BCNT = 2,
        DST   = 3,
        SRC_DST_BIDX = 4,
        LINK_BCNTRLD = 5,
        SRC_DST_CIDX = 6,
        CCNT  = 7  // По умолчанию в TRM AM335x
    };

    class QdmaChannel
    {
    private:
        uint8_t qch_num;
        uint8_t tcc_num;
        uint32_t param_id;
        QdmaTrigWord trig_word;
        REGS::EDMA::e_EVENT_QUEUE queue;
        bool is_allocated{false};

    public:
        explicit QdmaChannel(const uint8_t qdma_channel,
                             const uint8_t tcc,
                             const QdmaTrigWord trigger = QdmaTrigWord::CCNT,
                             const REGS::EDMA::e_EVENT_QUEUE q = REGS::EDMA::EVENT_Q0) noexcept
            : qch_num(qdma_channel), tcc_num(tcc), param_id(32 + qdma_channel),
              trig_word(trigger), queue(q) {}

        ~QdmaChannel() noexcept
        {
            if (is_allocated)
            {
                InterruptDispatcher::unregisterHandler(tcc_num);
                free();
            }
        }

        QdmaChannel(const QdmaChannel&) = delete;
        QdmaChannel& operator=(const QdmaChannel&) = delete;

        QdmaChannel(QdmaChannel&& other) noexcept
            : qch_num(other.qch_num), tcc_num(other.tcc_num), param_id(other.param_id),
              trig_word(other.trig_word), queue(other.queue), is_allocated(other.is_allocated)
        {
            other.is_allocated = false;
        }

        QdmaChannel& operator=(QdmaChannel&& other) noexcept
        {
            if (this != &other)
            {
                if (is_allocated)
                {
                    InterruptDispatcher::unregisterHandler(tcc_num);
                    free();
                }
                qch_num = other.qch_num;
                tcc_num = other.tcc_num;
                param_id = other.param_id;
                trig_word = other.trig_word;
                queue = other.queue;
                is_allocated = other.is_allocated;

                other.is_allocated = false;
            }
            return *this;
        }

        void setCallback(const Callback_t on_complete, const ErrorCallback_t on_error = nullptr, void* context = nullptr) const noexcept
        {
            InterruptDispatcher::registerHandler(tcc_num, on_complete, on_error, context);
        }

        bool init() noexcept
        {
            is_allocated = HAL::EDMA::request_channel(REGS::EDMA::CHANNEL_TYPE_QDMA,
                                                       qch_num,
                                                       tcc_num,
                                                       queue);
            if (!is_allocated) return false;

            HAL::EDMA::map_QDMA_ch_to_paRAM(qch_num, &param_id);
            HAL::EDMA::set_QDMA_trig_word(qch_num, static_cast<uint8_t>(trig_word));

            return true;
        }

        void free() noexcept
        {
            if (is_allocated)
            {
                HAL::EDMA::disable_QDMA_event(qch_num);
                HAL::EDMA::free_channel(REGS::EDMA::CHANNEL_TYPE_QDMA,
                                        qch_num,
                                        REGS::EDMA::TRIG_MODE_QDMA,
                                        tcc_num,
                                        queue);
                is_allocated = false;
            }
        }

        void configure(const REGS::EDMA::paRAM_entry_t& param) const noexcept
        {
            using namespace REGS::EDMA;

            AM335X_EDMA3CC->S_QEECR(e_REGION_ID::REGION_0).reg = (1u << qch_num);

            auto* dst = reinterpret_cast<volatile uint32_t*>(&AM335X_EDMA3CC->paRAM(param_id));
            const auto* src = reinterpret_cast<const uint32_t*>(&param);

            for (uint32_t i = 0; i < 8; ++i)
                dst[i] = src[i];

            __asm__ volatile("dmb" ::: "memory");

            AM335X_EDMA3CC->S_QEESR(e_REGION_ID::REGION_0).reg = (1u << qch_num);

            const auto trig_idx = static_cast<uint32_t>(trig_word);
            dst[trig_idx] = src[trig_idx];   // повторная запись
        }

        void start() const noexcept
        {
            HAL::EDMA::set_QDMA_trig_word(qch_num, static_cast<uint8_t>(trig_word));
        }

        // Quick Restart: update only the trigger word (for example, CCNT or the source/destination address)
        void triggerEntry(const uint32_t entry_index, const uint32_t val) const noexcept
        {
            HAL::EDMA::QDMA_set_paRAM_entry(param_id, entry_index, val);
        }

        [[nodiscard]] uint32_t getParamId() const noexcept { return param_id; }
    };
}

#endif // HAL_QDMACHANNEL_HPP