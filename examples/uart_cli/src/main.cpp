#include "init.h"
#include "hal/boards/beaglebone_black.hpp"
#include "rtt/rtt_log.h"
#include "shell/shell.hpp"
#include "tests/uart_tests.hpp"

#define TAG "uart_cli"

int main()
{
    if (!init_board())
    {
        RTT_LOG_E(TAG, "Board initialization failed");
        while (true)
            __asm volatile("wfi");
    }

    auto& uart = Board::get_uart0();
    SHELL::Shell shell{uart};
    UART_TESTS::register_commands(shell, uart);
    RTT_LOG_I(TAG, "UART CLI initialized");
    uart.put_string("\nAM335x UART CLI\n");
    UART_TESTS::print_help(uart);
    shell.run();
}
