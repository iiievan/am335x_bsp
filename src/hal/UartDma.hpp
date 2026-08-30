#ifndef HAL_UART_DMA_HPP
#define HAL_UART_DMA_HPP

#include <cstddef>
#include <cstdint>

#include "hal/CACHE.hpp"
#include "hal/UART.hpp"
#include "hal/EDMA/DmaChannel.hpp"
#include "hal/EDMA/ParamBuilder.hpp"
#include "startup/cp15.h"

namespace HAL::UART
{
    /**
     * Blocking UART/EDMA adapter.
     *
     * The UART owns request generation; this object owns EDMA channels and
     * their PaRAM configuration. HAL::EDMA::init() must be called beforehand.
     */
    template<typename Uart,
             uint8_t TxChannel,
             uint8_t RxChannel,
             uint8_t DummyParam>
    class UartDma final
    {
        Uart& m_uart;
        HAL::EDMA::DmaChannel m_tx{TxChannel};
        HAL::EDMA::DmaChannel m_rx{RxChannel};
        uint8_t m_tx_chunk;
        uint8_t m_rx_chunk;
        bool m_initialized{false};

        static uint32_t address_of(const void* ptr) noexcept
        {
            return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr));
        }

    public:
        UartDma(Uart& uart, const uint8_t tx_chunk = 8u,
                const uint8_t rx_chunk = 8u) noexcept
            : m_uart(uart), m_tx_chunk(tx_chunk), m_rx_chunk(rx_chunk)
        {}

        ~UartDma() noexcept
        {
            stop();
        }

        UartDma(const UartDma&) = delete;
        UartDma& operator=(const UartDma&) = delete;
        UartDma(UartDma&&) = delete;
        UartDma& operator=(UartDma&&) = delete;

        [[nodiscard]] bool init() noexcept
        {
            if (m_initialized)
                return true;
            if (m_uart.io_mode() != IOMode::DMA ||
                m_tx_chunk == 0u || m_rx_chunk == 0u ||
                TxChannel == RxChannel || DummyParam == TxChannel ||
                DummyParam == RxChannel)
            {
                return false;
            }

            if (!m_tx.init())
                return false;
            if (!m_rx.init())
            {
                m_tx.free();
                return false;
            }

            m_initialized = true;
            return true;
        }

        void stop() noexcept
        {
            if (!m_initialized)
                return;
            (void)m_tx.stop();
            (void)m_rx.stop();
            m_tx.free();
            m_rx.free();
            m_uart.DMA_disable();
            m_initialized = false;
        }

        /** Send complete chunks with EDMA and a possible tail by polling. */
        [[nodiscard]] bool transmit(const void* data, const size_t size,
                                    const uint32_t timeout_loops = 5'000'000u) noexcept
        {
            if (!m_initialized || (data == nullptr && size != 0u))
                return false;
            if (size == 0u)
                return true;

            const size_t dma_size = size - (size % m_tx_chunk);
            const size_t chunk_count = dma_size / m_tx_chunk;
            if (chunk_count > 0xFFFFu)
                return false;

            if (dma_size != 0u)
            {
                const auto source = reinterpret_cast<uintptr_t>(data);
                const auto param = HAL::EDMA::ParamBuilder()
                    .setSource(source, 1, m_tx_chunk)
                    .setDest(Uart::tx_dma_address(), 0, 0)
                    .setTransferParams(1u, m_tx_chunk,static_cast<uint16_t>(chunk_count))
                    .setSyncType(true)
                    .enableCompletionInterrupt(TxChannel)
                    .linkTo(DummyParam)
                    .build();
                const auto dummy = HAL::EDMA::PaRAMFactory::makeDummy(TxChannel);

                if (!m_tx.configure(REGS::EDMA::PaRAMConfig{TxChannel, param},
                                    REGS::EDMA::PaRAMConfig{DummyParam, dummy}))
                    return false;

                HAL::CACHE::dcache_clean_range(address_of(data),static_cast<uint32_t>(dma_size));

                cp15_DSB_barrier();

                // Переход DMA mode 0 → DMA mode 1 создаст новый UART request.
                m_uart.DMA_enable(REGS::UART::SCR_DMA_MODE_1);

                if (!m_tx.trigger(REGS::EDMA::TRIG_MODE_EVENT))
                {
                    m_uart.DMA_disable();
                    cp15_DSB_barrier();
                    (void)m_tx.stop();
                    return false;
                }

                if (!m_tx.wait_completion(timeout_loops))
                {
                    m_uart.DMA_disable();
                    cp15_DSB_barrier();
                    (void)m_tx.stop();
                    return false;
                }

                m_uart.DMA_disable();
                cp15_DSB_barrier();
                (void)m_tx.stop();
            }

            const auto* tail = static_cast<const uint8_t*>(data) + dma_size;

            for (std::size_t i = dma_size; i < size; ++i)
                m_uart.put_char(static_cast<char>(*tail++));

            // EDMA completion only means that the final bytes reached the UART
            // FIFO.  Wait until the transmitter has put them on the wire before
            // the caller is allowed to reconfigure or reset the UART.
            m_uart.wait_tx_complete();
            return true;
        }

        /** Receive complete FIFO-trigger-sized chunks with EDMA. */
        [[nodiscard]] bool receive(void* data, const size_t size,
                                   const uint32_t timeout_loops = 5'000'000u) noexcept
        {
            if (!m_initialized || (data == nullptr && size != 0u))
                return false;
            if (size == 0u)
                return true;
            if ((size % m_rx_chunk) != 0u || (size / m_rx_chunk) > 0xFFFFu)
                return false;

            const auto destination = reinterpret_cast<uintptr_t>(data);
            const auto param = HAL::EDMA::ParamBuilder()
                .setSource(Uart::rx_dma_address(), 0, 0)
                .setDest(destination, 1, m_rx_chunk)
                .setTransferParams(1u, m_rx_chunk,static_cast<uint16_t>(size / m_rx_chunk))
                .setSyncType(true)
                .enableCompletionInterrupt(RxChannel)
                // Peripheral DMA PaRAM must remain non-static even when LINK is null.
                // With STATIC=1, multi-event UART RX does not reach normal completion.
                .setStatic(false)
                .setLink(0xFFFFu)
                .build();

            if (!m_rx.configure(param))
                return false;

            HAL::CACHE::dcache_clean_invalidate_range(address_of(data),
                                                       static_cast<uint32_t>(size));
            cp15_DSB_barrier();
            m_uart.DMA_enable(REGS::UART::SCR_DMA_MODE_1);

            if (!m_rx.trigger(REGS::EDMA::TRIG_MODE_EVENT) ||
                !m_rx.wait_completion(timeout_loops))
            {
                (void)m_rx.stop();
                return false;
            }

            (void)m_rx.stop();
            cp15_DSB_barrier();
            HAL::CACHE::dcache_invalidate_range(address_of(data),
                                                 static_cast<uint32_t>(size));
            cp15_DSB_barrier();
            return true;
        }
    };

    using Uart0Dma = UartDma<uart0_t,
                             REGS::EDMA::CH_UART0_TX,
                             REGS::EDMA::CH_UART0_RX,
                             63u>;
}

#endif // HAL_UART_DMA_HPP
