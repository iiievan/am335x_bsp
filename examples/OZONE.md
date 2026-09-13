# Ozone project settings

Every `examples/*/ozone/*.jdebug` file starts with the same three user options:

```c
OZONE_RTT_ENABLED = 1;
OZONE_JLINK_LOG_ENABLED = 0;
OZONE_CONSOLE_LOG_ENABLED = 0;
```

- `OZONE_RTT_ENABLED` controls the Ozone RTT terminal. Keep it enabled for the
  DDR examples. The bootloader defaults to `0`, matching
  `AM335X_BOOT_LOG_RTT=OFF`; set both options to `1` when building bootloader
  RTT support.
- `OZONE_JLINK_LOG_ENABLED` writes the detailed J-Link DLL log beside the
  `.jdebug` project. Enable it only while diagnosing target access.
- `OZONE_CONSOLE_LOG_ENABLED` writes the Ozone console log beside the project.

RTT is disabled during connect, reset and download, then enabled from
`OnStartupComplete()`. On AM335x this workflow relies on the stop-mode
background-memory-access fallback in Ozone 3.38g with J-Link 8.32. Selecting a
CoreSight background-memory AP does not add hardware support to Cortex-A8.

The Debug and RelWithDebInfo projects differ only in the ELF path and log file
names. FreeRTOS additionally loads `FreeRTOSPlugin_CA9`; DDR applications use
`tools/AM335x_DDR_Init.jlinkscript`.
