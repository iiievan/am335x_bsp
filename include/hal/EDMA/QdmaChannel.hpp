#ifndef HAL_QDMACHANNEL_HPP
#define HAL_QDMACHANNEL_HPP

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
        REGS::EDMA::e_paRAM_entry_field trig_word_field;
        REGS::EDMA::e_EVENT_QUEUE queue;
        bool is_allocated{false};

        volatile bool m_transfer_done{false};
        volatile bool m_transfer_error{false};
        ErrorType     m_last_error{};

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
        explicit QdmaChannel(const uint8_t qdma_channel,
                             const uint8_t tcc,
                             const REGS::EDMA::e_paRAM_entry_field trigger = REGS::EDMA::e_paRAM_entry_field::CCNT,
                             const REGS::EDMA::e_EVENT_QUEUE q = REGS::EDMA::EVENT_Q0) noexcept
            : qch_num(qdma_channel), tcc_num(tcc), param_id(32 + qdma_channel),
              trig_word_field(trigger), queue(q) {}

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
              trig_word_field(other.trig_word_field), queue(other.queue), is_allocated(other.is_allocated),
              m_transfer_done(other.m_transfer_done), m_transfer_error(other.m_transfer_error),
              m_last_error(other.m_last_error)
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
                trig_word_field = other.trig_word_field;
                queue = other.queue;
                is_allocated = other.is_allocated;
                m_transfer_done = other.m_transfer_done;
                m_transfer_error = other.m_transfer_error;
                m_last_error = other.m_last_error;

                other.is_allocated = false;
            }
            return *this;
        }

        bool init() noexcept
        {
            is_allocated = request_channel(REGS::EDMA::CHANNEL_TYPE_QDMA,
                                                       qch_num,
                                                       tcc_num,
                                                       queue);
            if (!is_allocated) return false;

            map_QDMA_ch_to_paRAM(qch_num, param_id);
            set_QDMA_trig_word(qch_num, static_cast<uint8_t>(trig_word_field));

            InterruptDispatcher::registerHandler(tcc_num, m_on_complete,
                                                          m_on_error,
                                                  this);

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

        void configure(const REGS::EDMA::paRAM_entry_t& param) const noexcept
        {
            disable_QDMA_event(qch_num);
            QDMA_clr_miss_evt(qch_num);

            QDMA_set_paRAM(param_id, param);
            enable_QDMA_event(qch_num);
        }

        void trigger() noexcept
        {
            m_transfer_done = false;
            m_transfer_error = false;
            const auto val = QDMA_get_paRAM_entry(param_id,static_cast<uint32_t>(trig_word_field));
            QDMA_set_paRAM_entry(param_id, static_cast<uint32_t>(trig_word_field), val);
        }

        // Blocking wait
        [[nodiscard]] bool wait_completion(uint32_t timeout_loops = 5'000'000) noexcept
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