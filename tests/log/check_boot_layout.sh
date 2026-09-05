#!/usr/bin/env bash
set -euo pipefail
test_root="$(cd "$(dirname "$0")/../.." && pwd)"
test_build="$(mktemp -d)"
cd "$test_root"
# Test GNU ld region arithmetic on the host; this is not an ARM firmware build.
for mode in on off; do
    if [[ "$mode" == on ]]; then
        sram=0xFC00; rtt_origin=0x40300000; rtt_length=0x10000
    else
        sram=0x1FC00; rtt_origin=0x40310000; rtt_length=0x0
    fi
    sed -e "s/@BOOT_SRAM_LENGTH@/$sram/g" \
        -e "s/@BOOT_RTT_ORIGIN@/$rtt_origin/g" \
        -e "s/@BOOT_RTT_LENGTH@/$rtt_length/g" \
        -e '/OUTPUT_FORMAT/d' -e '/OUTPUT_ARCH/d' \
        examples/bootloader/am335x_boot.ld.in > "$test_build/$mode.ld"
done
gcc -fno-asynchronous-unwind-tables -c tests/log/boot_layout_probe.c -o "$test_build/plain.o"
gcc -fno-asynchronous-unwind-tables -DPROBE_RTT -c tests/log/boot_layout_probe.c -o "$test_build/rtt.o"
gcc -fno-asynchronous-unwind-tables -DPROBE_LARGE -c tests/log/boot_layout_probe.c -o "$test_build/large.o"
ld -T "$test_build/on.ld" "$test_build/rtt.o" -o "$test_build/on.elf"
ld -T "$test_build/off.ld" "$test_build/plain.o" -o "$test_build/off.elf"
ld -T "$test_build/off.ld" "$test_build/large.o" -o "$test_build/large.elf"
if ld -T "$test_build/off.ld" "$test_build/rtt.o" -o "$test_build/bad.elf" 2> "$test_build/rtt-error"; then
    echo 'FAIL: RTT accepted with zero reservation'; exit 1
fi
if ld -T "$test_build/on.ld" "$test_build/large.o" -o "$test_build/bad.elf" 2> "$test_build/size-error"; then
    echo 'FAIL: oversized SRAM accepted with RTT enabled'; exit 1
fi
if readelf -SW "$test_build/off.elf" | grep -q '\.rtt'; then
    echo 'FAIL: empty RTT output section was retained'; exit 1
fi
echo 'PASS: RTT on/off layout, reclaimed SRAM, RTT-off rejection, SRAM overflow guard'
