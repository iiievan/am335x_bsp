#ifndef AM335X_RTT_LOG_CAPI_COMPAT_HPP
#define AM335X_RTT_LOG_CAPI_COMPAT_HPP

#include "log/log_time.h"

#define rtt_get_system_time_ms log_get_system_time_ms

#ifdef __cplusplus
extern "C" {
#endif

static inline void rtt_time_init(void) { }

#ifdef __cplusplus
}
#endif

#endif
