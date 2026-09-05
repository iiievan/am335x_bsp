# Step 3: bootloader without RTT

Default: `AM335X_BOOT_LOG_RTT=OFF`, `AM335X_BOOT_LOG_UART=ON`.
Keep `AM335X_BSP_ENABLE_RTT_BACKEND=ON`: other examples still use RTT.
The SEGGER library can be built for those examples without being linked into
the bootloader. Its presence in the build directory is not a memory allocation
in the bootloader ELF.

| Configuration | SRAM available to code/data/stacks | Reserved RTT region |
| --- | ---: | ---: |
| Boot RTT ON | 0xFC00 = 64512 bytes | 0x10000 = 65536 bytes |
| Boot RTT OFF | 0x1FC00 = 130048 bytes | 0 bytes |

RTT ON uses SRAM [0x402F0400, 0x40300000), RTT [0x40300000, 0x40310000).
RTT OFF uses SRAM [0x402F0400, 0x40310000). Unlike the old overlapping regions,
these allocations cannot silently collide. The 6400-byte RTT payload/control
allocation and the reserved 64-KiB region are different quantities. This step
retains the conservative 64-KiB reservation when RTT is enabled; it does not
claim 64-KiB MMU granularity is mandatory. Bootloader disables MMU/caches.

`am335x_boot.ld.in` is the source template. CMake generates `am335x_boot.ld` in
the bootloader build directory. Do not edit the generated file. The old source
`am335x_boot.ld` has been replaced by the template; remove the old file when
overlaying an archive onto an existing checkout (it is no longer used).
RTT_BUFFER_SIZE is now the actual reservation (0 or 0x10000), not an unused
symbol or SEGGER buffer capacity. Linker assertions reject any nonempty .rtt
when disabled and enforce SRAM bounds independently of RTT's location.
An RTT-off post-build check runs arm-none-eabi-nm and rejects SEGGER RTT symbols.

## Hardware test

1. Add `-DAM335X_BOOT_LOG_RTT=OFF -DAM335X_BOOT_LOG_UART=ON` to the CLion CMake
   profile; reload CMake and rebuild am335x_boot. Leave global RTT backend ON.
2. Expect `Bootloader: no SEGGER RTT symbols`. Memory usage should show SRAM
   capacity 130048 bytes (127 KiB), RTT_RAM 0 B used / 0 B capacity (some linkers
   print an undefined percentage for an empty region).
3. Start `tio -b 115200 /dev/ttyUSB0` and run the newly built ELF in Ozone.
   UART must report DDR/board success and two LOG_TEST records with
   `Outputs RTT=0 UART=1`. The LEDs should continue blinking.
4. RTT will have no new records from this firmware. Clear any old RTT viewer
   history; old RAM contents can still contain a stale SEGGER signature after a
   warm debug restart. Disable RTT capture in Ozone if it discovers stale data.
5. Optional regression: build with `AM335X_BOOT_LOG_RTT=ON`; both channels should
   carry LOG_TEST records with `RTT=1`. SRAM is then limited to 64512 bytes. If
   it overflows, inspect the map/code footprint; do not restore overlapping
   MEMORY regions to bypass the error.

Early logs before UART initialization are dropped with RTT OFF; there is no
early-message buffer. Legacy RTT_LOG calls still route through the common LOG
frontend. Turning both boot channels OFF produces no physical log output but
does not compile away formatting call sites; BSP_LOG_COMPILED_LEVEL controls
that separately for the shared BSP build. No UART ISR logging is added.

Host validation: `bash tests/log/check_boot_layout.sh` and
`bash tests/log/run_host_tests.sh`. ARM compilation, real ELF size and board
execution still require the target toolchain/hardware.
