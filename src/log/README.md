# AM335x logging

`log/log.h` is the transport-independent logging frontend. It owns message
levels, timestamps, tag filtering and formatting. Output transports register a
`log_sink_t`; one formatted message is then sent to every registered sink.

## Compatibility

Existing `rtt/rtt_log.h` includes remain supported. `RTT_LOG_*` macros map to
the generic `LOG_*` frontend, while `rtt_log_init()` initializes the optional
RTT sink.

New code should include `log/log.h` and use `LOG_E`, `LOG_W`, `LOG_I`, `LOG_D`
and `LOG_T`.

## Build configuration

- `AM335X_BSP_ENABLE_RTT_BACKEND=ON|OFF` controls whether the RTT transport is
  built. The core BSP never links SEGGER RTT directly.
- `BSP_LOG_COMPILED_LEVEL` controls which calls are compiled: `0`/`1` disables
  all messages, `2` keeps errors, `3` warnings, `4` info, `5` debug and `6`
  trace.

Firmware using RTT must link `am335x::log_rtt`. Merely linking the BSP does not
pull in SEGGER RTT or allocate an RTT control block/buffer.

## Tag filtering

Up to 32 tags of 15 characters can be excluded at runtime. The filter uses
fixed storage and never allocates from the heap.
