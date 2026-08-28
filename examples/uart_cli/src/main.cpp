#include <cstddef>
#include <cstdint>

#include "init.h"
#include "hal/UartDma.hpp"
#include "hal/boards/beaglebone_black.hpp"
#include "line_editor.hpp"
#include "rtt/rtt_log.h"

#define TAG "uart_cli"

namespace
{
    using Uart = HAL::UART::uart0_t;

    constexpr std::size_t TEST_DATA_SIZE = 8u;
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
        if (!g_interrupt_active || c == '\r' || c == '\n')
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

    void print_test_result(const Uart& uart, const char* name, const bool passed) noexcept
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
        uart.put_string("Send POLL1234: ");

        std::size_t count = 0u;
        while (count < TEST_DATA_SIZE)
        {
            const char c = uart.get_char();
            if (c == '\r' || c == '\n')
                continue;
            buffer[count++] = c;
            uart.put_char(c);
        }
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
        uart.put_string("\n[INTERRUPT] TX uses polling; RX uses the UART ISR.\n");
        uart.put_string("Send IRQ12345: ");

        std::size_t echoed = 0u;
        uint32_t timeout = TEST_TIMEOUT_LOOPS;
        while (!g_interrupt_complete && timeout != 0u)
        {
            const std::size_t available = g_interrupt_count;
            while (echoed < available)
                uart.put_char(g_interrupt_buffer[echoed++]);
            --timeout;
            __asm volatile("nop");
        }
        g_interrupt_active = false;

        while (echoed < g_interrupt_count)
            uart.put_char(g_interrupt_buffer[echoed++]);
        uart.put_string("\n");

        if (!g_interrupt_complete || g_interrupt_count != TEST_DATA_SIZE)
            return false;

        char received[TEST_DATA_SIZE]{};
        for (std::size_t i = 0u; i < TEST_DATA_SIZE; ++i)
            received[i] = g_interrupt_buffer[i];
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

            constexpr char prompt[] =
                "\r\n[DMA] TX and RX use UART0 EDMA channels.\r\n"
                "Send DMA12345: ";
            constexpr char newline[] = "\r\n";

            if (!uart_dma.transmit(prompt, sizeof(prompt) - 1u, TEST_TIMEOUT_LOOPS) ||
                !uart_dma.receive(rx_buffer, TEST_DATA_SIZE, TEST_TIMEOUT_LOOPS) ||
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

    void print_help(const Uart& uart) noexcept
    {
        uart.put_string(
            "\nCommands:\n"
            "  help             Show this help\n"
            "  test polling     Polling TX + polling RX\n"
            "  test interrupt   Polling TX + ISR RX\n"
            "  test dma         EDMA TX + EDMA RX\n"
            "  test all         Run all three tests\n"
            "\nEditing:\n"
            "  Up/Down          Command history\n"
            "  Left/Right       Move cursor\n"
            "  Home/End         Start/end of line\n"
            "  Backspace/Delete Edit line\n"
            "  Ctrl+C           Cancel line\n"
            "  Ctrl+L           Clear screen\n");
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

    [[noreturn]] void run_cli(Uart& uart) noexcept
    {
        char command[LineEditor::COMMAND_BUFFER_SIZE]{};
        LineEditor editor{uart};

        uart.put_string("\nAM335x UART CLI\n");
        print_help(uart);
        uart.put_string("\n");

        while (true)
        {
            if (editor.read_line(command) == LineEditor::Result::SUBMITTED)
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
    ::run_cli(Board::get_uart0());

    while (true)
        __asm volatile("wfi");
}
