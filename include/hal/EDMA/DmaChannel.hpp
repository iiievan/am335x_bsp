
#ifndef HAL_DMACHANNEL_HPP
#define HAL_DMACHANNEL_HPP

#include "regs/EDMA.hpp"
#include "hal/EDMA/EDMA.hpp"
#include "InterruptDispatcher.hpp"

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
          is_allocated(other.is_allocated)
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
            is_allocated = HAL::EDMA::request_channel(REGS::EDMA::CHANNEL_TYPE_DMA,
                                                     ch_num,
                                                     tcc_num,
                                                     queue);
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

        void configure(const REGS::EDMA::paRAM_entry_t& param) const noexcept { HAL::EDMA::set_paRAM(ch_num, param); }

        void start(const TriggerMode mode = TriggerMode::TRIG_MODE_MANUAL) const noexcept
        { HAL::EDMA::enable_transfer(ch_num, mode); }

        void stop(const TriggerMode mode = TriggerMode::TRIG_MODE_MANUAL) const noexcept
        { HAL::EDMA::disable_transfer(ch_num, mode); }

        // Blocking wait for bare-metal tests
        void waitCompletion() const noexcept
        {
            if (ch_num < 32)
            {
                while (!(HAL::EDMA::get_intr_status() & (1u << ch_num))) {}
                HAL::EDMA::clr_intr(ch_num);
            }
            else
            {
                while (!(HAL::EDMA::intr_status_high_get() & (1u << (ch_num - 32)))) {}
                HAL::EDMA::clr_intr(ch_num);
            }
        }
    };
}

#endif //HAL_DMACHANNEL_HPP