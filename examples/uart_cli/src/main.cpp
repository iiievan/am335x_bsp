#include <cstddef>
#include <cstdint>

#include "init.h"
#include "hal/UartDma.hpp"
#include "hal/boards/beaglebone_black.hpp"
#include "rtt/rtt_log.h"

#define TAG "uart_cli"

namespace
{
    using Uart = HAL::UART::uart0_t;

    constexpr std::size_t TEST_DATA_SIZE = 8u;
    constexpr std::size_t COMMAND_BUFFER_SIZE = 48u;
    constexpr uint32_t TEST_TIMEOUT_LOOPS = 500'000'000u;
    constexpr char POLLING_PATTERN[] = "POLL1234";
    constexpr char INTERRUPT_PATTERN[] = "IRQ12345";
    constexpr char DMA_PATTERN[] = "DMA12345";

    volatile char g_interrupt_buffer[TEST_DATA_SIZE]{};
    volatile std::size_t g_interrupt_count{0u};
    volatile bool g_interrupt_complete{false};
    volatile bool g_interrupt_active{false};

    class PollingRestore final
    {
    public:
        explicit PollingRestore(Uart& uart) noexcept : m_uart(uart) {}
        ~PollingRestore() noexcept { m_uart.init_polling(); }

        PollingRestore(const PollingRestore&) = delete;
        PollingRestore& operator=(const PollingRestore&) = delete;

    private:
        Uart& m_uart;
    };

    [[nodiscard]] bool strings_equal(const char* lhs, const char* rhs) noexcept
    {
        while (*lhs != '\0' && *rhs != '\0')
        {
            if (*lhs++ != *rhs++)
                return false;
        }
        return *lhs == *rhs;
    }

    [[nodiscard]] bool buffer_equals(const char* buffer,
                                     const char* expected,
                                     const std::size_t size) noexcept
    {
        for (std::size_t i = 0u; i < size; ++i)
        {
            if (buffer[i] != expected[i])
                return false;
        }
        return expected[size] == '\0';
    }

    void interrupt_rx_callback(const char c)
    {
        if (!g_interrupt_active)
            return;

        const std::size_t index = g_interrupt_count;
        if (index < TEST_DATA_SIZE)
        {
            g_interrupt_buffer[index] = c;
            g_interrupt_count = index + 1u;
        }

        if (g_interrupt_count == TEST_DATA_SIZE)
        {
            g_interrupt_active = false;
            g_interrupt_complete = true;
        }
    }

    void reset_interrupt_receiver() noexcept
    {
        g_interrupt_active = false;
        g_interrupt_complete = false;
        g_interrupt_count = 0u;
        for (std::size_t i = 0u; i < TEST_DATA_SIZE; ++i)
            g_interrupt_buffer[i] = '\0';
    }

    void print_test_result(Uart& uart, const char* name, const bool passed) noexcept
    {
        uart.put_string("[RESULT] ");
        uart.put_string(name);
        uart.put_string(passed ? ": PASS\n" : ": FAIL\n");

        if (passed)
            RTT_LOG_I(TAG, "%s test passed", name);
        else
            RTT_LOG_E(TAG, "%s test failed", name);
    }

    [[nodiscard]] bool test_polling(Uart& uart) noexcept
    {
        PollingRestore restore{uart};
        uart.init_polling();
        char buffer[TEST_DATA_SIZE]{};

        uart.put_string("\n[POLLING] TX uses polling; RX uses polling.\n");
        uart.put_string("Type exactly POLL1234 (do not press Enter): ");
        for (std::size_t i = 0u; i < TEST_DATA_SIZE; ++i)
            buffer[i] = uart.get_char();

        uart.put_string("\n[POLLING] Echo: ");
        uart.put_data(buffer, TEST_DATA_SIZE);
        uart.put_string("\n");
        return buffer_equals(buffer, POLLING_PATTERN, TEST_DATA_SIZE);
    }

    [[nodiscard]] bool test_interrupt(Uart& uart) noexcept
    {
        PollingRestore restore{uart};
        reset_interrupt_receiver();

        if (!uart.init_interrupt(interrupt_rx_callback))
            return false;

        g_interrupt_active = true;
        // The current interrupt mode implements only ISR RX. TX is polling.
        uart.put_string("\n[INTERRUPT] TX uses polling; RX uses the UART ISR.\n");
        uart.put_string("Type exactly IRQ12345 (do not press Enter): ");

        uint32_t timeout = TEST_TIMEOUT_LOOPS;
        while (!g_interrupt_complete && timeout != 0u)
        {
            --timeout;
            __asm volatile("nop");
        }
        g_interrupt_active = false;

        if (!g_interrupt_complete || g_interrupt_count != TEST_DATA_SIZE)
            return false;

        char received[TEST_DATA_SIZE]{};
        for (std::size_t i = 0u; i < TEST_DATA_SIZE; ++i)
            received[i] = g_interrupt_buffer[i];

        uart.put_string("\n[INTERRUPT] Echo (polling TX): ");
        uart.put_data(received, TEST_DATA_SIZE);
        uart.put_string("\n");
        return buffer_equals(received, INTERRUPT_PATTERN, TEST_DATA_SIZE);
    }

    [[nodiscard]] bool test_dma(Uart& uart) noexcept
    {
        PollingRestore restore{uart};
        if (!uart.init_dma())
            return false;

        alignas(64) uint8_t rx_buffer[64]{};
        bool passed = false;

        {
            HAL::UART::Uart0Dma uart_dma{uart};
            if (!uart_dma.init())
                return false;

            constexpr char header[] =
                "\r\n[DMA] TX and RX use UART0 EDMA channels.\r\n"
                "Type exactly DMA12345 (do not press Enter): ";
            constexpr char echo_prefix[] = "\r\n[DMA] Echo: ";
            constexpr char newline[] = "\r\n";

            if (!uart_dma.transmit(header, sizeof(header) - 1u, TEST_TIMEOUT_LOOPS) ||
                !uart_dma.receive(rx_buffer, TEST_DATA_SIZE, TEST_TIMEOUT_LOOPS) ||
                !uart_dma.transmit(echo_prefix, sizeof(echo_prefix) - 1u, TEST_TIMEOUT_LOOPS) ||
                !uart_dma.transmit(rx_buffer, TEST_DATA_SIZE, TEST_TIMEOUT_LOOPS) ||
                !uart_dma.transmit(newline, sizeof(newline) - 1u, TEST_TIMEOUT_LOOPS))
            {
                return false;
            }

            passed = buffer_equals(reinterpret_cast<const char*>(rx_buffer),
                                   DMA_PATTERN,
                                   TEST_DATA_SIZE);
        }
        return passed;
    }

    void print_help(Uart& uart) noexcept
    {
        uart.put_string(
            "\nCommands:\n"
            "  help             Show this help\n"
            "  test polling     Polling TX + polling RX\n"
            "  test interrupt   Polling TX + ISR RX\n"
            "  test dma         EDMA TX + EDMA RX\n"
            "  test all         Run all three tests\n");
    }

    void run_all_tests(Uart& uart) noexcept
    {
        const bool polling_passed = test_polling(uart);
        print_test_result(uart, "polling", polling_passed);
        const bool interrupt_passed = test_interrupt(uart);
        print_test_result(uart, "interrupt (ISR RX + polling TX)", interrupt_passed);
        const bool dma_passed = test_dma(uart);
        print_test_result(uart, "dma", dma_passed);

        uart.put_string("\n[SUMMARY]\n");
        uart.put_string(polling_passed ? "  polling:   PASS\n" : "  polling:   FAIL\n");
        uart.put_string(interrupt_passed ? "  interrupt: PASS\n" : "  interrupt: FAIL\n");
        uart.put_string(dma_passed ? "  dma:       PASS\n" : "  dma:       FAIL\n");
        uart.put_string(polling_passed && interrupt_passed && dma_passed
                            ? "  overall:   PASS\n"
                            : "  overall:   FAIL\n");
    }

    void execute_command(Uart& uart, const char* command) noexcept
    {
        if (strings_equal(command, "help"))
            print_help(uart);
        else if (strings_equal(command, "test polling"))
            print_test_result(uart, "polling", test_polling(uart));
        else if (strings_equal(command, "test interrupt"))
            print_test_result(uart, "interrupt (ISR RX + polling TX)", test_interrupt(uart));
        else if (strings_equal(command, "test dma"))
            print_test_result(uart, "dma", test_dma(uart));
        else if (strings_equal(command, "test all"))
            run_all_tests(uart);
        else if (*command != '\0')
            uart.put_string("Unknown command. Type 'help'.\n");
    }

    void run_cli(Uart& uart) noexcept
    {
        char command[COMMAND_BUFFER_SIZE]{};
        bool ignore_lf = false;

        uart.put_string("\nAM335x UART CLI\n");
        print_help(uart);

        while (true)
        {
            uart.put_string("\nuart> ");
            std::size_t length = 0u;
            bool discard_line = false;
            command[0] = '\0';

            while (true)
            {
                const char c = uart.get_char();
                if (ignore_lf && c == '\n')
                {
                    ignore_lf = false;
                    continue;
                }
                ignore_lf = false;

                if (c == '\r' || c == '\n')
                {
                    ignore_lf = c == '\r';
                    uart.put_string("\n");
                    break;
                }

                if (c == '\b' || static_cast<uint8_t>(c) == 0x7Fu)
                {
                    if (!discard_line && length != 0u)
                    {
                        command[--length] = '\0';
                        uart.put_string("\b \b");
                    }
                    continue;
                }

                if (discard_line)
                    continue;
                if (length + 1u >= COMMAND_BUFFER_SIZE)
                {
                    discard_line = true;
                    continue;
                }

                command[length++] = c;
                command[length] = '\0';
                uart.put_char(c);
            }

            if (discard_line)
                uart.put_string("Command is too long.\n");
            else
                execute_command(uart, command);
        }
    }
}

int main()
{
    if (!init_board())
    {
        RTT_LOG_E(TAG, "Board initialization failed");
        while (true)
            __asm volatile("wfi");
    }

    RTT_LOG_I(TAG, "UART CLI initialized");
    run_cli(Board::get_uart0());

    while (true)
        __asm volatile("wfi");
}
