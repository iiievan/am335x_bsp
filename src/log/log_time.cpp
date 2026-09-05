#include "log/log_time.h"
#include "hal/sysTimer.hpp"

extern "C" uint64_t log_get_system_time_ms(void)
{
    return HAL::TIMERS::sys_time.get_ms();
}
