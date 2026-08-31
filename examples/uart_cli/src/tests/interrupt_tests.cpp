#include "uart_tests.hpp"

namespace UART_TESTS
{
    namespace
    {
        constexpr std::size_t DATA_SIZE = 8u;
        constexpr uint32_t TIMEOUT_LOOPS = 500'000'000u;
        constexpr char PATTERN[] = "IRQ12345";
        volatile char g_buffer[DATA_SIZE]{};
        volatile std::size_t g_count{0u};
        volatile bool g_complete{false};
        volatile bool g_active{false};

        void receive(const char value)
        {
            if (!g_active || value == '\r' || value == '\n')
                return;
            const std::size_t index = g_count;
            if (index < DATA_SIZE)
            {
                g_buffer[index] = value;
                g_count = index + 1u;
            }
            if (g_count == DATA_SIZE)
            {
                g_active = false;
                g_complete = true;
            }
        }

        void reset() noexcept
        {
            g_active = false;
            g_complete = false;
            g_count = 0u;
            for (std::size_t i = 0u; i < DATA_SIZE; ++i)
                g_buffer[i] = '\0';
        }
    }

    bool run_interrupt(Uart& uart) noexcept
    {
        reset();
        if (!uart.init_interrupt(receive))
        {
            uart.init_polling();
            return false;
        }
        g_active = true;
        uart.put_string("\n[INTERRUPT] TX uses polling; RX uses the UART ISR.\n");
        uart.put_string("Send IRQ12345: ");
        std::size_t echoed = 0u;
        uint32_t timeout = TIMEOUT_LOOPS;
        while (!g_complete && timeout != 0u)
        {
            while (echoed < g_count)
                uart.put_char(g_buffer[echoed++]);
            --timeout;
            __asm volatile("nop");
        }
        g_active = false;
        while (echoed < g_count)
            uart.put_char(g_buffer[echoed++]);
        uart.put_string("\n");
        const bool complete = g_complete && g_count == DATA_SIZE;
        bool equal = complete;
        for (std::size_t i = 0u; i < DATA_SIZE && equal; ++i)
            equal = g_buffer[i] == PATTERN[i];
        uart.init_polling();
        return equal;
    }
}
