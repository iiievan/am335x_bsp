
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
        bool is_allocated {false};
        bool is_configured{false};

        TriggerMode   m_trigger_mode     {TriggerMode::TRIG_MODE_MANUAL};
                 bool m_transfer_enabled {false};   // объект запускался и должен выполнить cleanup при освобождении
        volatile bool m_transfer_done    {false};
        volatile bool m_transfer_error   {false};
        ErrorType     m_last_error       {};

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
            : ch_num(channel),
              tcc_num(channel),
              queue(q) {}

        ~DmaChannel() noexcept
        {
            free();
        }

        // Allow only move (RAII)
        DmaChannel(const DmaChannel&) = delete;
        DmaChannel& operator=(const DmaChannel&) = delete;
        DmaChannel(DmaChannel&&) = delete;
        DmaChannel& operator=(DmaChannel&&) = delete;

        bool init(Callback_t onComplete = nullptr,
                  ErrorCallback_t onError = nullptr,
                  void* context = nullptr) noexcept
        {
            using namespace REGS::EDMA;
            if (tcc_num >= AM335X_DMACH_MAX ||
                static_cast<uint32_t>(queue) >= AM335x_TCS_MAX)
            {
                return false;
            }

            is_allocated = HAL::EDMA::request_channel(REGS::EDMA::CHANNEL_TYPE_DMA,
                                                     ch_num,
                                                     tcc_num,
                                                     queue);
            if (!is_allocated) return false;

            const auto complete_cb = reinterpret_cast<Callback_t>(onComplete ? onComplete : m_on_complete);
            const auto error_cb = reinterpret_cast<ErrorCallback_t>(onError ? onError : m_on_error);
            void* ctx = context ? context : this;

            InterruptDispatcher::registerHandler(tcc_num, complete_cb, error_cb, ctx);

            is_configured = false;
            return is_allocated;
        }

        void free() noexcept
        {
            if (!is_allocated)
                return;

            HAL::EDMA::free_channel(REGS::EDMA::CHANNEL_TYPE_DMA,
                                   ch_num,
                                   m_trigger_mode,
                                   tcc_num,
                                   queue);
            cp15_DSB_barrier();

            InterruptDispatcher::unregisterHandler(tcc_num);

            is_allocated = false;
            is_configured = false;
            m_transfer_enabled = false;
        }

        [[nodiscard]]
        bool configure(const REGS::EDMA::paRAM_entry_t& param)  noexcept
        {
            if (!is_allocated)
                return false;

            is_configured  = false;

            HAL::EDMA::set_paRAM(ch_num, param);
            cp15_DSB_barrier();

            is_configured  = true;
            return true;
        }

        [[nodiscard]]
        bool configure(const std::initializer_list<REGS::EDMA::PaRAMConfig> configs) noexcept
        {
            using namespace REGS::EDMA;

            if (!is_allocated || configs.size() == 0u)
                return false;

            is_configured = false;
            bool mapped_param_found = false;

            for (const auto& cfg : configs)
            {
                if (cfg.param_id >= AM335x_PARAMSETS_MAX)
                    return false;
                if (cfg.param_id == ch_num)
                    mapped_param_found = true;
            }

            if (!mapped_param_found)
                return false;

            for (const auto& cfg : configs)
            {
                HAL::EDMA::set_paRAM(cfg.param_id, cfg.entry);
            }

            cp15_DSB_barrier();
            is_configured = true;
            return true;
        }

        template<typename... Configs>
        [[nodiscard]]
        bool configure(const REGS::EDMA::PaRAMConfig& first, const Configs&... rest) noexcept
        {
            return configure({first, rest...});
        }

        bool trigger(const TriggerMode mode = TriggerMode::TRIG_MODE_MANUAL) noexcept
        {
            if (!is_allocated || !is_configured)
                return false;

            if (m_transfer_enabled && mode != m_trigger_mode)
            {
                if (!stop())
                    return false;
            }
            m_last_error    = {};
            m_transfer_done = false;
            m_transfer_error = false;

            cp15_DMB_barrier();
            if (!HAL::EDMA::enable_transfer(ch_num, mode)) return false;
            cp15_DSB_barrier();

            m_trigger_mode     = mode;
            m_transfer_enabled = true;
            return true;
        }

        bool stop() noexcept
        {
            if (!is_allocated) return false;
            if (!m_transfer_enabled) return true;

            const bool result = HAL::EDMA::disable_transfer(ch_num, m_trigger_mode);

            cp15_DSB_barrier();

            if (result) m_transfer_enabled = false;

            return result;
        }

        // Blocking wait
        [[nodiscard]] bool wait_completion(uint32_t timeout_loops = 5'000'000) const noexcept
        {
            while (!m_transfer_done && timeout_loops != 0u)
            {
                --timeout_loops;
                asm volatile("nop");
            }
            return m_transfer_done && !m_transfer_error;
        }

        [[nodiscard]] bool is_busy() const noexcept { return !m_transfer_done; }
        [[nodiscard]] bool has_error() const noexcept { return m_transfer_error; }
    };
}

#endif //HAL_DMACHANNEL_HPP