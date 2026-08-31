#include "uart_tests.hpp"

namespace UART_TESTS
{
    namespace
    {
        constexpr std::size_t DATA_SIZE = 8u;
        constexpr char PATTERN[] = "POLL1234";

        [[nodiscard]] bool equals(const char* data, const char* expected) noexcept
        {
            for (std::size_t i = 0u; i < DATA_SIZE; ++i)
                if (data[i] != expected[i])
                    return false;
            return expected[DATA_SIZE] == '\0';
        }
    }

    bool run_polling(Uart& uart) noexcept
    {
        uart.init_polling();
        char buffer[DATA_SIZE]{};
        uart.put_string("\n[POLLING] TX uses polling; RX uses polling.\n");
        uart.put_string("Send POLL1234: ");
        std::size_t count = 0u;
        while (count < DATA_SIZE)
        {
            const char value = uart.get_char();
            if (value == '\r' || value == '\n')
                continue;
            buffer[count++] = value;
            uart.put_char(value);
        }
        uart.put_string("\n");
        return equals(buffer, PATTERN);
    }
}
