#ifndef AM335X_LOG_LOG_H
#define AM335X_LOG_LOG_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    LOG_LEVEL_DEFAULT = 0,
    LOG_LEVEL_NONE    = 1,
    LOG_LEVEL_ERROR   = 2,
    LOG_LEVEL_WARN    = 3,
    LOG_LEVEL_INFO    = 4,
    LOG_LEVEL_DEBUG   = 5,
    LOG_LEVEL_TRACE   = 6,
} log_level_t;

typedef void (*log_sink_write_fn)(void* context, log_level_t level,
                                  const char* data, size_t size);

typedef struct
{
    void* context;
    log_sink_write_fn write;
} log_sink_t;

int log_register_sink(const log_sink_t* sink);
void log_unregister_sink(const log_sink_t* sink);

void log_print_format(log_level_t level, const char* tag,
                      const char* format, ...)
    __attribute__((__format__(__printf__, 3, 4)));

void log_set_level(log_level_t level);
log_level_t log_get_level(void);
int log_exclude_tag(const char* tag);
void log_unexclude_tag(const char* tag);

#ifndef LOG_COMPILED_LEVEL
#define LOG_COMPILED_LEVEL 6
#endif

#if LOG_COMPILED_LEVEL >= 2
#define LOG_E(tag, format, ...) \
    log_print_format(LOG_LEVEL_ERROR, tag, format, ##__VA_ARGS__)
#else
#define LOG_E(tag, format, ...) ((void)0)
#endif

#if LOG_COMPILED_LEVEL >= 3
#define LOG_W(tag, format, ...) \
    log_print_format(LOG_LEVEL_WARN, tag, format, ##__VA_ARGS__)
#else
#define LOG_W(tag, format, ...) ((void)0)
#endif

#if LOG_COMPILED_LEVEL >= 4
#define LOG_I(tag, format, ...) \
    log_print_format(LOG_LEVEL_INFO, tag, format, ##__VA_ARGS__)
#else
#define LOG_I(tag, format, ...) ((void)0)
#endif

#if LOG_COMPILED_LEVEL >= 5
#define LOG_D(tag, format, ...) \
    log_print_format(LOG_LEVEL_DEBUG, tag, format, ##__VA_ARGS__)
#else
#define LOG_D(tag, format, ...) ((void)0)
#endif

#if LOG_COMPILED_LEVEL >= 6
#define LOG_T(tag, format, ...) \
    log_print_format(LOG_LEVEL_TRACE, tag, format, ##__VA_ARGS__)
#else
#define LOG_T(tag, format, ...) ((void)0)
#endif

#define LOG_REG(level, module_name, reg_name, reg_value) \
    LOG_##level(#module_name, "%s = 0x%08X", #reg_name, \
                (unsigned int)(reg_value))

#define LOG_CHECK_MODULE_SIZE(module_type, last_reg_offset) \
do { \
    const size_t expected_size = (last_reg_offset) + 4u; \
    const size_t actual_size = sizeof(module_type); \
    if (actual_size != expected_size) { \
        LOG_E("module_size", \
              "%s size mismatch! Expected: 0x%08X, Actual: 0x%08X (diff: %d bytes)", \
              #module_type, (unsigned int)expected_size, \
              (unsigned int)actual_size, \
              (int)(actual_size - expected_size)); \
    } else { \
        LOG_D("module_size", "%s size OK: 0x%08X", #module_type, \
              (unsigned int)actual_size); \
    } \
} while (0)

#ifdef NDEBUG
#define LOG_ASSERT(expr) ((void)0)
#else
#define LOG_ASSERT(expr) \
do { \
    if (!(expr)) { \
        LOG_E("ASSERT", "Assertion failed: %s, file %s, line %d", \
              #expr, __FILE__, __LINE__); \
        while (1) { } \
    } \
} while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif
