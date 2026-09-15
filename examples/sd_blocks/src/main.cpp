#include <stdbool.h>
#include <stdint.h>

#include "init.h"
#include "log/log.h"
#include "sd_blocks.h"

#define TAG "main"

int main()
{
    if (!init_board())
    {
        LOG_E(TAG, "Board initialization failed");

        while (true)
        {
            __asm volatile("wfi");
        }
    }

    LOG_I(TAG, "Board initialization done");
    LOG_I(TAG, "Starting raw SD block test on MMC0 (microSD slot)");

    if (sd_blocks_test())
        LOG_I(TAG, "SD block test finished successfully");
    else
        LOG_E(TAG, "SD block test failed");

    while (true)
    {
        __asm volatile("wfi");
    }
}
