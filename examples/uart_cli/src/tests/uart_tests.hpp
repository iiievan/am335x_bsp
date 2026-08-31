#ifndef UART_CLI_TESTS_HPP
#define UART_CLI_TESTS_HPP

#include "hal/UART.hpp"
#include "shell/shell.hpp"

namespace UART_TESTS
{
    using Uart = HAL::UART::uart0_t;

    [[nodiscard]] bool run_polling(Uart& uart) noexcept;
    [[nodiscard]] bool run_interrupt(Uart& uart) noexcept;
    [[nodiscard]] bool run_dma(Uart& uart) noexcept;
    void register_commands(SHELL::Shell& shell, Uart& uart) noexcept;
    void print_help(Uart& uart) noexcept;
}

#endif
