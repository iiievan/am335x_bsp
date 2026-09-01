#ifndef HAL_UART_DETAIL_DMA_BACKEND_HPP
#define HAL_UART_DETAIL_DMA_BACKEND_HPP

#include <cstddef>
#include <cstdint>

#include "hal/CACHE.hpp"
#include "hal/EDMA/DmaChannel.hpp"
#include "hal/EDMA/ParamBuilder.hpp"
#include "startup/cp15.h"

namespace HAL::UART::detail
{
    template<class Owner, uint8_t TxChannel, uint8_t RxChannel,
             uint8_t DummyParam>
    class UartDmaBackend final
    {
    public:
        explicit UartDmaBackend(Owner& owner) noexcept : m_owner(owner) {}
        ~UartDmaBackend() noexcept { stop(); }

        UartDmaBackend(const UartDmaBackend&) = delete;
        UartDmaBackend& operator=(const UartDmaBackend&) = delete;
        UartDmaBackend(UartDmaBackend&&) = delete;
        UartDmaBackend& operator=(UartDmaBackend&&) = delete;

        [[nodiscard]] bool init(const DMAConfig& config) noexcept
        {
            if (m_initialized)
                return true;
            if (!valid(config))
                return false;
            if (!m_tx.init())
                return false;
            if (!m_rx.init())
            {
                m_tx.free();
                return false;
            }
            m_config = config;
            m_initialized = true;
            return true;
        }

        void stop() noexcept
        {
            if (!m_initialized)
                return;
            m_owner.DMA_disable();
            (void)m_tx.stop();
            (void)m_rx.stop();
            m_tx.free();
            m_rx.free();
            m_initialized = false;
        }

        [[nodiscard]] bool active() const noexcept { return m_initialized; }

        [[nodiscard]] bool write(const void* data, const std::size_t size,
                                 const uint32_t timeout_loops,
                                 const uint32_t timeout_epochs) noexcept
        {
            if (!m_initialized || (data == nullptr && size != 0u))
                return false;
            if (size == 0u)
                return true;

            const std::size_t dma_size = size - size % m_config.tx_chunk;
            const std::size_t chunks = dma_size / m_config.tx_chunk;
            if (chunks > 0xFFFFu)
                return false;

            if (dma_size != 0u)
            {
                const auto param = HAL::EDMA::ParamBuilder()
                    .setSource(reinterpret_cast<uintptr_t>(data),
                               1, m_config.tx_chunk)
                    .setDest(Owner::tx_dma_address(), 0, 0)
                    .setTransferParams(1u, m_config.tx_chunk,
                                       static_cast<uint16_t>(chunks))
                    .setSyncType(true)
                    .enableCompletionInterrupt(TxChannel)
                    .linkTo(DummyParam)
                    .build();
                const auto dummy =
                    HAL::EDMA::PaRAMFactory::makeDummy(TxChannel);
                if (!m_tx.configure(
                        REGS::EDMA::PaRAMConfig{TxChannel, param},
                        REGS::EDMA::PaRAMConfig{DummyParam, dummy}))
                {
                    return false;
                }

                HAL::CACHE::dcache_clean_range(address_of(data),
                                                static_cast<uint32_t>(dma_size));
                cp15_DSB_barrier();
                m_owner.DMA_enable(m_config.mode);
                if (!m_tx.trigger(REGS::EDMA::TRIG_MODE_EVENT) ||
                    !m_tx.wait_completion(timeout_loops, timeout_epochs))
                {
                    abort_tx();
                    return false;
                }
                m_owner.DMA_disable();
                cp15_DSB_barrier();
                (void)m_tx.stop();
            }

            const auto* tail = static_cast<const uint8_t*>(data) + dma_size;
            for (std::size_t i = dma_size; i < size; ++i)
                m_owner.put_char(static_cast<char>(*tail++));
            m_owner.wait_tx_complete();
            return true;
        }

        [[nodiscard]] bool read(void* data, const std::size_t size,
                                const uint32_t timeout_loops,
                                const uint32_t tail_timeout_loops,
                                const uint32_t timeout_epochs) noexcept
        {
            if (!m_initialized || (data == nullptr && size != 0u))
                return false;
            if (size == 0u)
                return true;

            const std::size_t dma_size = size - size % m_config.rx_chunk;
            const std::size_t chunks = dma_size / m_config.rx_chunk;
            if (chunks > 0xFFFFu)
                return false;

            if (dma_size != 0u)
            {
                const auto param = HAL::EDMA::ParamBuilder()
                    .setSource(Owner::rx_dma_address(), 0, 0)
                    .setDest(reinterpret_cast<uintptr_t>(data),
                             1, m_config.rx_chunk)
                    .setTransferParams(1u, m_config.rx_chunk,
                                       static_cast<uint16_t>(chunks))
                    .setSyncType(true)
                    .enableCompletionInterrupt(RxChannel)
                    .setStatic(false)
                    .setLink(0xFFFFu)
                    .build();
                if (!m_rx.configure(param))
                    return false;

                HAL::CACHE::dcache_clean_invalidate_range(
                    address_of(data), static_cast<uint32_t>(dma_size));
                cp15_DSB_barrier();
                m_owner.DMA_enable(m_config.mode);
                if (!m_rx.trigger(REGS::EDMA::TRIG_MODE_EVENT) ||
                    !m_rx.wait_completion(timeout_loops, timeout_epochs))
                {
                    abort_rx();
                    return false;
                }
                m_owner.DMA_disable();
                cp15_DSB_barrier();
                (void)m_rx.stop();
                HAL::CACHE::dcache_invalidate_range(
                    address_of(data), static_cast<uint32_t>(dma_size));
                cp15_DSB_barrier();
            }

            const std::size_t tail_size = size - dma_size;
            if (tail_size == 0u)
                return true;
            auto* tail = static_cast<uint8_t*>(data) + dma_size;
            return m_owner.read_dma_tail(tail, tail_size,
                                         tail_timeout_loops);
        }

    private:
        Owner& m_owner;
        HAL::EDMA::DmaChannel m_tx{TxChannel};
        HAL::EDMA::DmaChannel m_rx{RxChannel};
        DMAConfig m_config{};
        bool m_initialized{false};

        [[nodiscard]] static uint32_t address_of(const void* pointer) noexcept
        {
            return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pointer));
        }

        [[nodiscard]] static bool valid(const DMAConfig& config) noexcept
        {
            return TxChannel != 0xFFu && RxChannel != 0xFFu &&
                   TxChannel != RxChannel && DummyParam != TxChannel &&
                   DummyParam != RxChannel && config.tx_chunk != 0u &&
                   config.rx_chunk != 0u;
        }

        void abort_tx() noexcept
        {
            m_owner.DMA_disable();
            cp15_DSB_barrier();
            (void)m_tx.stop();
        }

        void abort_rx() noexcept
        {
            m_owner.DMA_disable();
            cp15_DSB_barrier();
            (void)m_rx.stop();
        }
    };
}

#endif
