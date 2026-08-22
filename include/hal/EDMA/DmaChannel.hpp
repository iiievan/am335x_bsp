
#ifndef HAL_DMACHANNEL_HPP
#define HAL_DMACHANNEL_HPP

#include <initializer_list>

#include "regs/EDMA.hpp"
#include "hal/EDMA/EDMA.hpp"
#include "InterruptDispatcher.hpp"
#include "startup/cp15.h"

namespace HAL::EDMA
{
    using TriggerMode = REGS::EDMA::e_EDMA3_TRIG_MODE_TYPE;

    class DmaChannel
    {
    private:
        uint8_t ch_num;
        uint8_t tcc_num;
        REGS::EDMA::e_EVENT_QUEUE queue;
        bool is_allocated{false};

        volatile bool m_transfer_done{false};
        volatile bool m_transfer_error{false};
        ErrorType     m_last_error{};

        static void m_on_complete(void* context) noexcept
        {
            auto* self = static_cast<DmaChannel*>(context);
            if (self) {
                self->m_transfer_done = true;
            }
        }

        static void m_on_error(ErrorType err, void* context) noexcept
        {
            auto* self = static_cast<DmaChannel*>(context);
            if (self) {
                self->m_last_error = err;
                self->m_transfer_error = true;
                self->m_transfer_done = true; // Завершаем ожидание при ошибке
            }
        }

    public:
        explicit DmaChannel(const uint8_t channel, const REGS::EDMA::e_EVENT_QUEUE q = REGS::EDMA::EVENT_Q0) noexcept
            : ch_num(channel), tcc_num(channel), queue(q) {}

        ~DmaChannel() noexcept
        {
            if (is_allocated)
            {
                InterruptDispatcher::unregisterHandler(tcc_num);
                free();
            }
        }

        // Allow only move (RAII)
        DmaChannel(const DmaChannel&) = delete;
        DmaChannel& operator=(const DmaChannel&) = delete;
        DmaChannel(DmaChannel&& other) noexcept
        : ch_num(other.ch_num), tcc_num(other.tcc_num), queue(other.queue),
        is_allocated(other.is_allocated), m_transfer_done(other.m_transfer_done),
        m_transfer_error(other.m_transfer_error), m_last_error(other.m_last_error)
        {
            other.is_allocated = false;
        }

        DmaChannel& operator=(DmaChannel&& other) noexcept
        {
            if (this != &other)
            {
                if (is_allocated)
                {
                    InterruptDispatcher::unregisterHandler(tcc_num);
                    free();
                }
                ch_num = other.ch_num;
                tcc_num = other.tcc_num;
                queue = other.queue;
                is_allocated = other.is_allocated;
                m_transfer_done = other.m_transfer_done;
                m_transfer_error = other.m_transfer_error;
                m_last_error = other.m_last_error;

                other.is_allocated = false;
            }
            return *this;
        }

        bool init(Callback_t onComplete = nullptr,
                  ErrorCallback_t onError = nullptr,
                  void* context = nullptr) noexcept
        {
            is_allocated = HAL::EDMA::request_channel(REGS::EDMA::CHANNEL_TYPE_DMA,
                                                     ch_num,
                                                     tcc_num,
                                                     queue);
            if (!is_allocated) return false;

            const auto complete_cb = reinterpret_cast<Callback_t>(onComplete ? onComplete : m_on_complete);
            const auto error_cb = reinterpret_cast<ErrorCallback_t>(onError ? onError : m_on_error);
            void* ctx = context ? context : this;

            InterruptDispatcher::registerHandler(tcc_num, complete_cb, error_cb, ctx);

            return is_allocated;
        }

        void free() noexcept
        {
            if (is_allocated)
            {
                HAL::EDMA::free_channel(REGS::EDMA::CHANNEL_TYPE_DMA,
                                       ch_num,
                                       REGS::EDMA::TRIG_MODE_MANUAL,
                                       tcc_num,
                                       queue);
                is_allocated = false;
            }
        }

        void configure(const REGS::EDMA::paRAM_entry_t& param) const noexcept
        {
            HAL::EDMA::set_paRAM(ch_num, param);

            cp15_DSB_barrier();
        }

        void configure(const std::initializer_list<REGS::EDMA::PaRAMConfig> configs) const noexcept
        {
            for (const auto& cfg : configs)
            {
                HAL::EDMA::set_paRAM(cfg.param_id, cfg.entry);
            }

            cp15_DSB_barrier();
        }

        template <typename... Configs>
        void configure(const REGS::EDMA::PaRAMConfig& first, const Configs&... rest) const noexcept
        {
            configure({first, rest...});
        }

        void trigger(const TriggerMode mode = TriggerMode::TRIG_MODE_MANUAL) noexcept
        {
            m_transfer_done = false;
            m_transfer_error = false;

            cp15_DMB_barrier();
            HAL::EDMA::enable_transfer(ch_num, mode);
            cp15_DSB_barrier();
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

        [[nodiscard]] bool is_busy() const noexcept { return !m_transfer_done; }
        [[nodiscard]] bool has_error() const noexcept { return m_transfer_error; }
    };
}

#endif //HAL_DMACHANNEL_HPP