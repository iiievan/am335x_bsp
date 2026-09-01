#include <cstdint>

#include "init.h"
#include "hal/UART.hpp"
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

    auto& uart = Board::get_uart0();
    if (!uart.init_dma())
    {
        RTT_LOG_E(TAG, "UART EDMA channel initialization failed");
        while (true) __asm volatile("wfi");
    }

    constexpr char welcome[] =
        "\r\nAM335x UART0 + EDMA example\r\n";

    constexpr char prompt[] =
        "Enter exactly 8 characters: ";

    constexpr char echo_prefix[] =
        "\r\nEcho: ";

    if (!uart.write(welcome, sizeof(welcome) - 1u))
    {
        RTT_LOG_E(TAG, "UART EDMA welcome transfer failed");
    }
    else
    {
        bool passed = true;

        for (uint32_t attempt = 0u; attempt < 2u; ++attempt)
        {
            if (!uart.write(prompt, sizeof(prompt) - 1u) ||
                !uart.read(rx_buffer, 8u, 500'000'000u) ||
                !uart.write(echo_prefix,sizeof(echo_prefix) - 1u) ||
                !uart.write(rx_buffer, 8u) ||
                !uart.write("\r\n", 2u))
            {
                RTT_LOG_E(TAG, "UART EDMA transfer %lu failed", static_cast<unsigned long>(attempt + 1u));

                passed = false;
                break;
            }

            RTT_LOG_I(TAG, "UART EDMA transfer %lu passed", static_cast<unsigned long>(attempt + 1u));
        }

        if (passed)
            RTT_LOG_I(TAG, "Both UART EDMA echo tests passed");

        RTT_LOG_I(TAG, "Entering idle loop");

        while (true)
            __asm volatile("wfi");
    }
}
