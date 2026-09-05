#include "init.h"
#include "rtt/rtt_log.h"
#include "hal/boards/beaglebone_black.hpp"
#include "hal/sysTimer.hpp"
#include "log/sinks/UartSink.hpp"


#define TAG "main"

void delay_ms(const uint32_t ms)
{
    using namespace HAL::TIMERS;
    const volatile uint32_t start = sys_time.get_ms();
    while((sys_time.get_ms() - start) < ms);
}

int main ()
{
#if AM335X_BOOT_LOG_UART
    // Registered before init_board, but silent until UART0 enters POLLING.
    // Lifetime covers board initialization and the entire foreground loop.
    HAL::LOG::UartSink uart_log{Board::get_uart0()};
#endif
    bool init_sts = false;

    init_sts = init_board();

    if (!init_sts)
    {
        RTT_LOG_E(TAG, "Board initialization failed!");
        while (1);
    }

    RTT_LOG_I(TAG, "Board initialization seccess!");
#if AM335X_BOOT_LOG_UART
    if (!uart_log.registered())
        LOG_E("LOG_TEST", "UART sink registration failed");
    else
    {
        LOG_I("LOG_TEST", "RTT + UART: shared record 1/2");
        delay_ms(10u);
        LOG_I("LOG_TEST", "RTT + UART: shared record 2/2");
    }
#endif

    uint8_t counter = 0;
    while(true)
    {
        delay_ms(150);
        switch (counter)
        {
            case 0:
                Board::USR0.toggle();
                counter++;
                break;
            case 1:
                Board::USR1.toggle();
                counter++;
                break;
            case 2:
                Board::USR2.toggle();
                counter++;
                break;
            case 3:
                Board::USR3.toggle();
                counter++;
                break;
            default:
                counter = 0;
                break;
        }
    }

    return(0);
}
