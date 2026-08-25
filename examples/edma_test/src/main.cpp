#include <stdint.h>
#include <stdbool.h>
#include "init.h"
#include "rtt/rtt_log.h"
#include "edma_test.h"
#include "hal/PERF.hpp"

#define TAG "main"

int main ()
{
    if (!init_board())
    {
        RTT_LOG_E(TAG, "Board initialization failed");

        while (true)
        {
            __asm volatile("wfi");
        }
    }

    RTT_LOG_I(TAG, "Board initialization done");
    RTT_LOG_I(TAG, "Starting standalone EDMA test");

    edma_test();

    RTT_LOG_I(TAG, "EDMA test finished");

    while (true)
    {
        __asm volatile("wfi");
    }
}

