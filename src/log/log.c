#include "log/log.h"
#include "log/log_time.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifndef LOG_MAX_SINK_COUNT
#define LOG_MAX_SINK_COUNT 4u
#endif

#ifndef LOG_MAX_TAG_COUNT
#define LOG_MAX_TAG_COUNT 32u
#endif

#ifndef LOG_MAX_TAG_LENGTH
#define LOG_MAX_TAG_LENGTH 16u
#endif

#ifndef LOG_MESSAGE_BUFFER_SIZE
#define LOG_MESSAGE_BUFFER_SIZE 512u
#endif

#ifndef LOG_DEFAULT_LEVEL
#define LOG_DEFAULT_LEVEL LOG_LEVEL_DEBUG
#endif

static log_level_t s_log_level = LOG_DEFAULT_LEVEL;
static const log_sink_t* s_sinks[LOG_MAX_SINK_COUNT];
static char s_excluded_tags[LOG_MAX_TAG_COUNT][LOG_MAX_TAG_LENGTH];

static int tag_is_excluded(const char* tag)
{
    size_t i;
    for (i = 0u; i < LOG_MAX_TAG_COUNT; ++i)
    {
        if (s_excluded_tags[i][0] != '\0' &&
            strcmp(s_excluded_tags[i], tag) == 0)
            return 1;
    }
    return 0;
}

static const char* level_letter(log_level_t level)
{
    switch (level)
    {
    case LOG_LEVEL_ERROR: return "E";
    case LOG_LEVEL_WARN:  return "W";
    case LOG_LEVEL_INFO:  return "I";
    case LOG_LEVEL_DEBUG: return "D";
    case LOG_LEVEL_TRACE: return "T";
    default:              return " ";
    }
}

int log_register_sink(const log_sink_t* sink)
{
    size_t i;
    if (sink == NULL || sink->write == NULL)
        return 0;

    for (i = 0u; i < LOG_MAX_SINK_COUNT; ++i)
        if (s_sinks[i] == sink)
            return 1;

    for (i = 0u; i < LOG_MAX_SINK_COUNT; ++i)
    {
        if (s_sinks[i] == NULL)
        {
            s_sinks[i] = sink;
            return 1;
        }
    }
    return 0;
}

void log_unregister_sink(const log_sink_t* sink)
{
    size_t i;
    for (i = 0u; i < LOG_MAX_SINK_COUNT; ++i)
    {
        if (s_sinks[i] == sink)
        {
            s_sinks[i] = NULL;
            return;
        }
    }
}

void log_print_format(log_level_t level, const char* tag,
                      const char* format, ...)
{
    char line[LOG_MESSAGE_BUFFER_SIZE];
    uint64_t timestamp;
    uint32_t seconds;
    uint32_t milliseconds;
    int prefix_size;
    int message_size;
    size_t line_size;
    size_t i;
    int has_sink = 0;
    va_list args;

    if (level == LOG_LEVEL_NONE || level > s_log_level ||
        tag == NULL || format == NULL)
        return;

    for (i = 0u; i < LOG_MAX_SINK_COUNT; ++i)
        has_sink |= s_sinks[i] != NULL;

    if (!has_sink || tag_is_excluded(tag))
        return;

    timestamp = log_get_system_time_ms();
    seconds = (uint32_t)(timestamp / 1000u);
    milliseconds = (uint32_t)(timestamp % 1000u);

    prefix_size = snprintf(line, sizeof(line), "[%5u.%03u]:\t[%s][%s]:\t",
                           (unsigned int)seconds,
                           (unsigned int)milliseconds,
                           level_letter(level), tag);
    if (prefix_size < 0)
        return;
    if ((size_t)prefix_size >= sizeof(line))
        prefix_size = (int)(sizeof(line) - 1u);

    va_start(args, format);
    message_size = vsnprintf(line + prefix_size,
                             sizeof(line) - (size_t)prefix_size,
                             format, args);
    va_end(args);
    if (message_size < 0)
        return;

    line_size = (size_t)prefix_size + (size_t)message_size;
    if (line_size >= sizeof(line) - 1u)
        line_size = sizeof(line) - 2u;
    line[line_size++] = '\n';
    line[line_size] = '\0';

    for (i = 0u; i < LOG_MAX_SINK_COUNT; ++i)
    {
        const log_sink_t* sink = s_sinks[i];
        if (sink != NULL)
            sink->write(sink->context, level, line, line_size);
    }
}

void log_set_level(log_level_t level)
{
    LOG_ASSERT(level <= LOG_LEVEL_TRACE);
    s_log_level = level == LOG_LEVEL_DEFAULT ? LOG_DEFAULT_LEVEL : level;
}

log_level_t log_get_level(void)
{
    return s_log_level;
}

int log_exclude_tag(const char* tag)
{
    size_t i;
    size_t length;

    LOG_ASSERT(tag != NULL);
    if (tag == NULL)
        return 0;

    length = strlen(tag);
    LOG_ASSERT(length < LOG_MAX_TAG_LENGTH);
    if (length >= LOG_MAX_TAG_LENGTH)
        return 0;
    if (tag_is_excluded(tag))
        return 1;

    for (i = 0u; i < LOG_MAX_TAG_COUNT; ++i)
    {
        if (s_excluded_tags[i][0] == '\0')
        {
            memcpy(s_excluded_tags[i], tag, length + 1u);
            return 1;
        }
    }
    return 0;
}

void log_unexclude_tag(const char* tag)
{
    size_t i;
    LOG_ASSERT(tag != NULL);
    if (tag == NULL)
        return;

    for (i = 0u; i < LOG_MAX_TAG_COUNT; ++i)
    {
        if (s_excluded_tags[i][0] != '\0' &&
            strcmp(s_excluded_tags[i], tag) == 0)
        {
            s_excluded_tags[i][0] = '\0';
            return;
        }
    }
}
