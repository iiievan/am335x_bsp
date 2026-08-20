#ifndef HAL_QDMACHANNEL_HPP
#define HAL_QDMACHANNEL_HPP

#include <initializer_list>

#include "regs/EDMA.hpp"
#include "hal/EDMA/EDMA.hpp"
#include "InterruptDispatcher.hpp"

namespace HAL::EDMA
{
    class QdmaChannel
    {
    private:
        uint8_t qch_num;
        uint8_t tcc_num;
        uint32_t param_id;
        REGS::EDMA::e_EVENT_QUEUE queue;
        bool is_allocated{false};

        static void m_on_complete(void* context) noexcept
        {
            auto* self = static_cast<QdmaChannel*>(context);
            if (self) {
                self->m_transfer_done = true;
            }
        }

        static void m_on_error(ErrorType err, void* context) noexcept
        {
            auto* self = static_cast<QdmaChannel*>(context);
            if (self) {
                self->m_last_error = err;
                self->m_transfer_error = true;
                self->m_transfer_done = true;
            }
        }

    public:

        REGS::EDMA::e_paRAM_entry_field trig_word_field;
        uint32_t trigger_word_value;

        volatile bool m_transfer_done{false};
        volatile bool m_transfer_error{false};
        ErrorType     m_last_error{};

        explicit QdmaChannel(const uint8_t qdma_channel,
                             const uint8_t tcc,
                             const REGS::EDMA::e_paRAM_entry_field trigger = REGS::EDMA::e_paRAM_entry_field::CCNT,
                             const REGS::EDMA::e_EVENT_QUEUE q = REGS::EDMA::EVENT_Q0) noexcept
            : qch_num(qdma_channel), tcc_num(tcc), param_id(32 + qdma_channel),
              queue(q), trig_word_field(trigger), trigger_word_value(0)
        { }

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
        QdmaChannel(QdmaChannel&&) = delete;
        QdmaChannel& operator=(QdmaChannel&&) = delete;


        bool init(Callback_t onComplete = nullptr,
                  ErrorCallback_t onError = nullptr,
                  void* context = nullptr) noexcept
        {
            is_allocated = request_channel(REGS::EDMA::CHANNEL_TYPE_QDMA,
                                                       qch_num,
                                                       tcc_num,
                                                       queue);
            if (!is_allocated) return false;

            map_QDMA_ch_to_paRAM(qch_num, param_id);
            set_QDMA_trig_word(qch_num, static_cast<uint8_t>(trig_word_field));

            const auto complete_cb = reinterpret_cast<Callback_t>(onComplete ? onComplete : m_on_complete);
            const auto error_cb = reinterpret_cast<ErrorCallback_t>(onError ? onError : m_on_error);
            void* ctx = context ? context : this;

            InterruptDispatcher::registerHandler(tcc_num, complete_cb, error_cb, ctx);

            return true;
        }

        void free() noexcept
        {
            if (is_allocated)
            {
                disable_QDMA_event(qch_num);
                REGS::EDMA::AM335X_EDMA3CC->QDMAQNUM.reg &=  REGS::EDMA::QDMAQNUM_CLR(qch_num);

                free_channel(REGS::EDMA::CHANNEL_TYPE_QDMA,
                                        qch_num,
                                        REGS::EDMA::TRIG_MODE_QDMA,
                                        tcc_num,
                                        queue);
                is_allocated = false;
            }
        }

        bool configure(const REGS::EDMA::paRAM_entry_t& param) noexcept
        {
            if (!disable_QDMA_event_and_wait(qch_num))
                return false;

            QDMA_clr_miss_evt(qch_num);

            constexpr uint32_t field_count = static_cast<uint32_t>(REGS::EDMA::e_paRAM_entry_field::paRAMfieldsMAX);

            static_assert(sizeof(REGS::EDMA::paRAM_entry_t) == field_count * sizeof(uint32_t));

            uint32_t words[field_count]{};
            __builtin_memcpy(words, &param, sizeof(param));

            const uint32_t trigger_index = static_cast<uint32_t>(trig_word_field);
            trigger_word_value = words[trigger_index];

            QDMA_set_paRAM(param_id, param, trig_word_field);

            __asm__ volatile("dsb" ::: "memory");
            enable_QDMA_event(qch_num);
            __asm__ volatile("dsb" ::: "memory");

            return is_QDMA_event_enabled(qch_num);
        }

        void trigger() noexcept
        {
            m_transfer_done  = false;
            m_transfer_error = false;
            m_last_error     = {};

            __asm__ volatile("dmb" ::: "memory");
            QDMA_set_paRAM_entry(param_id,static_cast<uint32_t>(trig_word_field),trigger_word_value);
            __asm__ volatile("dsb" ::: "memory");
        }

        void reset_flags() noexcept
        {
            m_transfer_done = false;
            m_transfer_error = false;
            m_last_error = {};
        }

        // Blocking wait
        [[nodiscard]] bool wait_completion(uint32_t timeout_loops = 5'000'000) const noexcept
        {
            while (!m_transfer_done && --timeout_loops)
            {
                asm volatile("nop");
            }
            return m_transfer_done && !m_transfer_error;
        }

        [[nodiscard]] uint32_t getParamId() const noexcept { return param_id; }
        [[nodiscard]] bool is_busy() const noexcept { return !m_transfer_done; }
        [[nodiscard]] bool has_error() const noexcept { return m_transfer_error; }
    };
}

#endif // HAL_QDMACHANNEL_HPP