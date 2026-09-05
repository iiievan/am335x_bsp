#include "log/sinks/RttSink.hpp"

#include "log/log.h"
#include "SEGGER_RTT.h"

#include <cstring>

namespace
{
    constexpr const char* COLOR_RESET = "\033[0m";
    constexpr const char* COLOR_ERROR = "\033[0;31m";
    constexpr const char* COLOR_WARN  = "\033[0;33m";
    constexpr const char* COLOR_INFO  = "\033[0;32m";
    constexpr const char* COLOR_DEBUG = "\033[0;34m";
    constexpr const char* COLOR_TRACE = "\033[0;35m";

    unsigned int s_users = 0u;

    const char* color_for(const log_level_t level) noexcept
    {
        switch (level)
        {
        case LOG_LEVEL_ERROR: return COLOR_ERROR;
        case LOG_LEVEL_WARN:  return COLOR_WARN;
        case LOG_LEVEL_INFO:  return COLOR_INFO;
        case LOG_LEVEL_DEBUG: return COLOR_DEBUG;
        case LOG_LEVEL_TRACE: return COLOR_TRACE;
        default:              return COLOR_RESET;
        }
    }

    void write_rtt(void*, const log_level_t level,
                   const char* data, const size_t size)
    {
        const char* color = color_for(level);
        SEGGER_RTT_Write(0u, color, static_cast<unsigned int>(std::strlen(color)));
        SEGGER_RTT_Write(0u, data, static_cast<unsigned int>(size));
        SEGGER_RTT_Write(0u, COLOR_RESET,
                         static_cast<unsigned int>(std::strlen(COLOR_RESET)));
    }

    const log_sink_t s_rtt_sink{nullptr, write_rtt};
}

bool HAL::LOG::rtt_backend_init() noexcept
{
    if (s_users != 0u)
    {
        ++s_users;
        return true;
    }

    SEGGER_RTT_Init();
    if (log_register_sink(&s_rtt_sink) == 0)
        return false;

    s_users = 1u;
    return true;
}

void HAL::LOG::rtt_backend_deinit() noexcept
{
    if (s_users == 0u)
        return;

    --s_users;
    if (s_users == 0u)
        log_unregister_sink(&s_rtt_sink);
}

extern "C" void rtt_log_init(void)
{
    static bool initialized = false;
    if (!initialized)
        initialized = HAL::LOG::rtt_backend_init();
}
