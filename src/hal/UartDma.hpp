#ifndef HAL_UART_DMA_HPP
#define HAL_UART_DMA_HPP

#include <cstddef>
#include <cstdint>

#include "hal/UART.hpp"

namespace HAL::UART
{
    /**
     * Compatibility adapter for code written before DMA ownership moved into
     * uart.  New applications should use uart::init_dma(), read() and write().
     */
    template<typename Uart, uint8_t TxChannel, uint8_t RxChannel,
             uint8_t DummyParam>
    class UartDma final
    {
        Uart& m_uart;
        uint8_t m_tx_chunk;
        uint8_t m_rx_chunk;

    public:
        explicit UartDma(Uart& uart, const uint8_t tx_chunk = 8u,
                         const uint8_t rx_chunk = 8u) noexcept
            : m_uart(uart), m_tx_chunk(tx_chunk), m_rx_chunk(rx_chunk)
        {}

        UartDma(const UartDma&) = delete;
        UartDma& operator=(const UartDma&) = delete;

        [[nodiscard]] bool init() noexcept
        {
            if (m_uart.io_mode() == IOMode::DMA)
                return true;
            DMAConfig config{};
            config.tx_chunk = m_tx_chunk;
            config.rx_chunk = m_rx_chunk;
            return m_uart.init_dma(config);
        }

        void stop() noexcept
        {
            if (m_uart.io_mode() == IOMode::DMA)
                m_uart.init_polling();
        }

        [[nodiscard]] bool transmit(const void* data, const std::size_t size,
                                    const uint32_t timeout_loops = 5'000'000u,
                                    const uint32_t timeout_epochs = 1u) noexcept
        {
            return m_uart.write(data, size, timeout_loops, timeout_epochs);
        }

        [[nodiscard]] bool receive(void* data, const std::size_t size,
                                   const uint32_t timeout_loops = 5'000'000u,
                                   const uint32_t tail_timeout_loops = 5'000'000u,
                                   const uint32_t timeout_epochs = 1u) noexcept
        {
            return m_uart.read(data, size, timeout_loops, tail_timeout_loops,
                               timeout_epochs);
        }
    };

    using Uart0Dma = UartDma<uart0_t, REGS::EDMA::CH_UART0_TX,
                             REGS::EDMA::CH_UART0_RX, 63u>;
}

#endif
