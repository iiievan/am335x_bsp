#include <stdbool.h>
#include <stdint.h>

#include "init.h"
#include "log/log.h"
#include "sd_fatfs.h"

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
    LOG_I(TAG, "Starting FatFs test on MMC0 (microSD slot)");

    if (sd_fatfs_test())
        LOG_I(TAG, "FatFs test finished successfully");
    else
        LOG_E(TAG, "FatFs test failed");

    while (true)
    {
        __asm volatile("wfi");
    }
}
