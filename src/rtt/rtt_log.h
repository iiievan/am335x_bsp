#ifndef AM335X_RTT_LOG_COMPAT_H
#define AM335X_RTT_LOG_COMPAT_H

#include "log/log.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Compatibility entry point supplied by the optional RTT backend. */
void rtt_log_init(void);

typedef log_level_t rtt_log_level_t;

#define RTTLogLevelDefault LOG_LEVEL_DEFAULT
#define RTTLogLevelNone    LOG_LEVEL_NONE
#define RTTLogLevelError   LOG_LEVEL_ERROR
#define RTTLogLevelWarn    LOG_LEVEL_WARN
#define RTTLogLevelInfo    LOG_LEVEL_INFO
#define RTTLogLevelDebug   LOG_LEVEL_DEBUG
#define RTTLogLevelTrace   LOG_LEVEL_TRACE

#define rtt_log_print_format  log_print_format
#define rtt_log_set_level     log_set_level
#define rtt_log_get_level     log_get_level
#define rtt_log_exclude_tag   log_exclude_tag
#define rtt_log_unexclude_tag log_unexclude_tag

#define RTT_LOG_E LOG_E
#define RTT_LOG_W LOG_W
#define RTT_LOG_I LOG_I
#define RTT_LOG_D LOG_D
#define RTT_LOG_T LOG_T
#define RTT_LOG_REG LOG_REG
#define RTT_CHECK_MODULE_SIZE LOG_CHECK_MODULE_SIZE
#define RTT_LOG_ASSERT LOG_ASSERT

#ifdef __cplusplus
}
#endif

#endif
