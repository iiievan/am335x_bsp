#!/usr/bin/env bash
set -euo pipefail
test_root="$(cd "$(dirname "$0")/../.." && pwd)"
test_build="$(mktemp -d)"
cd "$test_root"
gcc -std=c11 -Wall -Wextra -Werror -Isrc -c src/log/log.c -o "$test_build/log.o"
g++ -std=c++17 -Wall -Wextra -Werror -Isrc \
    -Ithirdparty/SEGGER_RTT/RTT -Ithirdparty/SEGGER_RTT/Config \
    tests/log/uart_sink_test.cpp src/log/sinks/RttSink.cpp "$test_build/log.o" \
    -o "$test_build/uart_sink_test"
"$test_build/uart_sink_test"
