# Unified logging in the FreeRTOS example

Default: `AM335X_FREERTOS_LOG_RTT=ON`, `AM335X_FREERTOS_LOG_UART=ON`.
These target-specific options do not change bootloader or UART CLI.
Keep global `AM335X_BSP_ENABLE_RTT_BACKEND=ON` while other examples need RTT.

All BSP and example source calls use `log/log.h` and `LOG_*`; plain UART boot
strings are replaced by structured logs. PERF/MMU use the same common
logger/lock as the application.

## Lifecycle

init_board cleans the RTT cache region (only when RTT enabled), then calls
App::Logging::init once. A static RAII Session owns the RTT/UART registrations.
UART registration does not touch hardware; init_board initializes UART0 only
when its logging option is ON. Early records are RTT-only; UART-only mode drops
them until the UART is ready. No replay buffer is allocated.

After board initialization, main calls prepare_scheduler to allocate a FreeRTOS
mutex from the existing FreeRTOS heap. Allocation is intentionally delayed until
INTC is ready, because FreeRTOS queue/mutex creation uses port critical sections.
Task creation is checked; on failure scheduler startup is aborted.
The session lives for firmware lifetime; do not destroy it with active tasks.

## Task safety and limits

The new optional log_set_lock callbacks protect formatting, timestamp sampling,
and the entire dispatch to all sinks. Baremetal examples install no callbacks
and retain their existing behavior. Before scheduler start, logging is direct.
After startup, FreeRTOS tasks take the mutex with priority inheritance; IRQs
remain enabled during polling UART output. Slow UART output still blocks the
logging task and other log producers. This is synchronous logging, not a queue.

IRQ/FIQ/Abort/Undefined, suspended scheduler, and IRQ-masked task contexts drop
records before formatting. The actual port's ulPortInterruptNesting is checked:
IRQ dispatch switches to SVC mode, so CPSR mode alone is not a reliable ISR test.
**Existing Abort LOG_E call sites no longer emit through this logger.** An
independent emergency logging path is future work; don't use this API in hooks
that run with interrupts masked. This policy avoids deadlocking on a task mutex.

Register/unregister sinks, change filters/levels, and configure lock callbacks
only before starting tasks or after all log producers have been stopped.
Do not call LOG recursively from a sink. Don't use the UART TX path separately
from this logger while tasks run. The common mutex cannot serialize unrelated
newlib printf users; configUSE_NEWLIB_REENTRANT is unchanged.

LED task stack depths increase from 512 to 2048 StackType_t words for formatter/
newlib headroom. This is a conservative allowance, not a measured stack bound.
LED logs now correctly identify USR0 and USR1, use separate TAGs, and express
delays through pdMS_TO_TICKS. The unused polling delay helper was removed.

## Hardware check

1. Build am335x_freertos with both log options ON; open Ozone RTT and
   `tio -b 115200 /dev/ttyUSB0`.
2. Expect `Logging ready: RTT=1 UART=1`, board success, then PERF, LED0 and LED1
   records. Compare text/timestamps per record between channels. UART uses CRLF
   and RTT ANSI colors. The system timer starts with the scheduler: early 0.000
   timestamps are normal; task timestamps must advance.
3. Let it run at least 30 seconds: both LEDs continue, records remain complete,
   no freeze. Logging affects benchmark timing, especially with UART enabled.
4. Repeat RTT-only (UART OFF), UART-only (RTT OFF), then both OFF if desired.
   UART is not initialized when its option is OFF. Both OFF is silent; LED tasks
   still run. This does not compile away call arguments; use the shared
   BSP_LOG_COMPILED_LEVEL setting separately for that purpose.

This step does not change the FreeRTOS linker map or reclaim its RTT region.
Host tests: bash tests/log/run_host_tests.sh. They check serialized dispatch
with two host threads and existing sink regression. They do not execute the
AM335x port, FreeRTOS scheduling, CPU-mode checks or MMIO: hardware validation
remains required.
