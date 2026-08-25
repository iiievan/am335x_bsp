#include <cstdint>

#include "init.h"
#include "hal/UartDma.hpp"
#include "hal/boards/beaglebone_black.hpp"
#include "rtt/rtt_log.h"

#define TAG "uart_dma"

namespace
{
    alignas(64) uint8_t rx_buffer[64]{};

    constexpr char welcome[] =
        "\r\nAM335x UART0 + EDMA example\r\n"
        "Enter exactly 8 characters: ";
}

int main()
{
    if (!init_board())
    {
        RTT_LOG_E(TAG, "Board initialization failed");
        while (true) __asm volatile("wfi");
    }

    HAL::UART::Uart0Dma uart_dma{Board::get_uart0()};
    if (!uart_dma.init())
    {
        RTT_LOG_E(TAG, "UART EDMA channel initialization failed");
        while (true) __asm volatile("wfi");
    }

    if (!uart_dma.transmit(welcome, sizeof(welcome) - 1u) ||
        !uart_dma.receive(rx_buffer, 8u, 500'000'000u) ||
        !uart_dma.transmit("\r\nEcho: ", 9u) ||
        !uart_dma.transmit(rx_buffer, 8u) ||
        !uart_dma.transmit("\r\n", 2u))
    {
        RTT_LOG_E(TAG, "UART EDMA transfer failed");
    }
    else
    {
        RTT_LOG_I(TAG, "UART EDMA echo passed");
    }

    while (true) __asm volatile("wfi");
}
