#include "uart_tests.hpp"

namespace UART_TESTS
{
    namespace
    {
        constexpr std::size_t DATA_SIZE = 8u;
        constexpr uint32_t TIMEOUT_LOOPS = 500'000'000u;
        constexpr char PATTERN[] = "DMA12345";

        [[nodiscard]] uint32_t timeout_epochs(const Uart& uart) noexcept
        {
            constexpr uint32_t rates[] = {
                300u, 600u, 1200u, 2400u, 4800u, 9600u, 14400u, 19200u,
                28800u, 38400u, 57600u, 115200u, 230400u, 460800u,
                921600u, 1843200u, 3686400u};
            const auto index = static_cast<uint32_t>(uart.baudrate());
            const uint32_t baud = index < sizeof(rates) / sizeof(rates[0])
                ? rates[index] : 115200u;
            const uint32_t ratio = (115200u + baud - 1u) / baud;
            return ratio < 4u ? 4u : ratio;
        }
    }

    bool run_dma(Uart& uart) noexcept
    {
        if (!uart.init_dma())
            return false;
        alignas(64) uint8_t buffer[64]{};
        constexpr char prompt[] =
            "\r\n[DMA] TX and RX use UART0 EDMA channels.\r\n"
            "Send DMA12345: ";
        constexpr char newline[] = "\r\n";
        const uint32_t epochs = timeout_epochs(uart);
        bool passed = uart.write(prompt, sizeof(prompt) - 1u,
                                 TIMEOUT_LOOPS, epochs) &&
                      uart.read(buffer, DATA_SIZE, TIMEOUT_LOOPS,
                                TIMEOUT_LOOPS, epochs) &&
                      uart.write(buffer, DATA_SIZE, TIMEOUT_LOOPS, epochs) &&
                      uart.write(newline, sizeof(newline) - 1u,
                                 TIMEOUT_LOOPS, epochs);
        for (std::size_t i = 0u; i < DATA_SIZE && passed; ++i)
            passed = buffer[i] == static_cast<uint8_t>(PATTERN[i]);
        uart.init_polling();
        return passed;
    }
}
