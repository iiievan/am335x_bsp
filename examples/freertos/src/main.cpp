#include "init.h"
#include "log/log.h"
#include "logging.hpp"
#include "hal/boards/beaglebone_black.hpp"
#include "hal/sysTimer.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include "hal/PERF.hpp"

#define TAG "main"

void vTask1(void *pvParameters)
{
    (void)pvParameters;
    for(;;)
    {
        Board::USR0.toggle();
        LOG_I("LED0", "USR0.LED toggle!");
        vTaskDelay(pdMS_TO_TICKS(1250));
    }
}

void vTask2(void *pvParameters)
{
    (void)pvParameters;
    for(;;)
    {
        Board::USR1.toggle();
        LOG_I("LED1", "USR1.LED toggle!");
        vTaskDelay(pdMS_TO_TICKS(750));
    }
}

void vPerfBenchmarkTask(void *pvParameters)
{
    (void)pvParameters;

    // Configuring the PMU to Monitor Data TLB Misses and Branch Predictor Failures
    HAL::PERF::configure_event(HAL::PERF::Counter::COUNTER_0, HAL::PERF::EventType::L1D_TLB_REFILL);
    HAL::PERF::configure_event(HAL::PERF::Counter::COUNTER_1, HAL::PERF::EventType::BRANCH_MISPRED);

    // Running a synthetic RAM performance test
    HAL::PERF::run_ddr_benchmark();

    for(;;)
    {
        {
            HAL::PERF::ScopedProfiler prof("EDMA_EXEC");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main ()
{
    bool init_sts = false;

    init_sts = init_board();

    if (!init_sts)
    {
        LOG_E(TAG, "Board initialization failed!");
        while (true){}
    }
    LOG_I(TAG, "Board initialization done!");
    if (!App::Logging::prepare_scheduler())
    {
        LOG_E(TAG, "Logging mutex allocation failed; scheduler not started");
        for (;;) { }
    }

    // Stack depths are StackType_t words. Formatter + newlib need headroom.
    if (xTaskCreate(vPerfBenchmarkTask, "PerfTask", 8192, nullptr, 2, nullptr) != pdPASS ||
        xTaskCreate(vTask1, "Task1", 2048, nullptr, 1, nullptr) != pdPASS ||
        xTaskCreate(vTask2, "Task2", 2048, nullptr, 1, nullptr) != pdPASS)
    {
        LOG_E(TAG, "Task creation failed; scheduler not started");
        for (;;) { }
    }

    vTaskStartScheduler();
    LOG_E(TAG, "Scheduler returned unexpectedly");

    for(;;){} // Should never reach here

    return(0);
}
