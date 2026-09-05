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

## Step 2: polling UART sink

```cpp
#include "log/sinks/UartSink.hpp"

// Hardware is configured by the application, not the logger.
Board::get_uart0().init_polling();
HAL::LOG::UartSink uart_log{Board::get_uart0()};
if (uart_log.registered())
    LOG_I("APP", "Sent to every registered sink");
```

Keep the sink alive for as long as output is needed. It holds a non-owning
reference; the UART must outlive it. Copy/move are disabled because the core
stores the address of its registration. Destruction unregisters this sink only.
Do not register multiple sinks for the same UART.

The constructor is also safe before UART initialization: it only registers a
callback. Each callback checks `io_mode()` and skips output unless it is POLLING.
It never enables clocks, changes modes, or acquires EDMA channels. INTERRUPT is
deliberately excluded too; shared RX/CLI use needs explicit coordination.

`enable(false)` pauses UART output; RTT continues. `enable()` resumes it.
`delivered()` counts completed records; `dropped()` counts skipped/timed-out
records. A timeout can leave a partial record on the wire; no retry is performed.
The constructor's optional timeout argument (default 5,000,000) is a loop budget
per raw write, NOT milliseconds. Bare LF becomes CRLF, existing CRLF is preserved.
UART has no ANSI color prefix; timestamp, level, TAG and message match RTT.

### Execution contract

This first polling sink and the current registry require a single serialized
foreground context. Do not log via UART from ISR/abort/panic handlers or multiple
FreeRTOS tasks, and do not register/unregister/reconfigure UART concurrently with
logging. The local recursion guard is not a task/IRQ lock. Interrupts may remain
enabled when their handlers do not log/use this UART. Thread-safe queues and an
emergency transport are NOT implemented. Do not mask interrupts around a long
UART write just to serialize it. The existing formatter uses a 512-byte stack
buffer and newlib `vsnprintf`; its stack/code footprint remains relevant.
As a defensive measure on ARM, the UART callback skips IRQ/FIQ/Abort/Undefined
CPU modes without updating counters. This avoids polling UART in the existing
exception handlers, but does not make the common formatter/RTT path ISR-safe.

### Hardware smoke test

Bootloader alone enables this sink by default (`AM335X_BOOT_LOG_UART=ON`). RTT
remains connected. Registration happens in main before init_board, so early
messages are RTT-only until UART0 is initialized. Subsequent logs reach both.
Two `LOG_TEST` messages separated by delay_ms(10) verify shared text/timestamps.
RTT and UART timestamps should agree for each individual record; the second
record's time should advance. UART transmission itself adds elapsed time.
Existing plain UART boot messages are retained, so some information appears
twice, once plain and once as a structured log. That is intentional for regression.

Open `tio -b 115200 /dev/ttyUSB0`, run bootloader in Ozone, and compare `LOG_TEST`
records. To restore the previous UART output, configure
`-DAM335X_BOOT_LOG_UART=OFF` and rebuild bootloader. No call-site edits are needed.
UART CLI and all other examples are unchanged: binary autotests must not receive
UART log text. The CLI's uninitialized timestamp timer is not changed in this step.

RTT-off bootloader linking and OCMC repartitioning remain a separate step.
This change does not modify any linker script or claim to reclaim RTT memory.

Host checks (no board/toolchain required): `bash tests/log/run_host_tests.sh`.
The host test uses the real formatter and RTT sink with fake transports; it does
not validate MMIO timing, ARM ABI, or hardware timeouts. Check the ARM build's
size/map output separately, especially the bootloader's SRAM headroom.
