# UART0 CLI and automated tests

This bare-metal example verifies UART0 in three transport modes:

- polling TX and polling RX;
- polling TX and interrupt-driven RX;
- EDMA TX and EDMA RX on channels 26 and 27.

All three transports are selected on the same RAII `uart0_t` object. DMA is an
internal `hal/detail` backend; application code does not create or coordinate a
second channel-owning wrapper:

```cpp
auto& uart = Board::get_uart0();
uart.init_polling();
(void)uart.init_interrupt(on_byte);
(void)uart.init_dma();
(void)uart.read(buffer, size);
(void)uart.write(buffer, size);
```

Changing mode releases the previous mode's interrupt and EDMA resources before
acquiring the new ones. A failed DMA initialization rolls the object back to a
usable polling state. DMA implementation details live in
`hal/EDMA/detail/UartBackend.hpp`; applications do not include that header
directly.

## Source layout

The entry point only initializes the board, registers commands and starts the
shell. The reusable shell is independent of UART0 and accepts any console type
that provides `get_char`, `put_char`, `put_string` and `put_data`:

```text
src/
├── main.cpp
├── shell/
│   ├── console.hpp
│   ├── line_editor.hpp / line_editor.cpp
│   └── shell.hpp / shell.cpp
└── tests/
    ├── uart_tests.hpp
    ├── polling_tests.cpp
    ├── interrupt_tests.cpp
    ├── dma_tests.cpp
    └── test_protocol.cpp
```

Commands are registered as visible or hidden. Hidden automation commands are
accepted by the dispatcher but omitted from help and Tab completion.

## Interactive CLI

The interactive interface intentionally contains only five commands:

- `help`
- `test polling`
- `test interrupt`
- `test dma`
- `test all`

Each test asks for an eight-character token, echoes it and prints `PASS` or
`FAIL`. UART0 always returns to polling mode before control returns to the CLI.

The line editor has an eight-entry history and uses no dynamic allocation.
Up/Down browse history; Left/Right, Home, End, Backspace and Delete edit the
line. Ctrl+C cancels it, Ctrl+L clears the terminal, and Tab completes a
command.

Run `run_tio.sh` to map Enter to one LF byte and Delete to Backspace. If tio is
connected after the initial prompt was printed, restart the target or press
Enter to obtain a fresh prompt.

## Fast UART acceptance test

Install the only host dependency, flash the current example, disconnect tio,
and run:

```bash
python3 -m pip install -r tools/requirements.txt
python3 tools/uart_autotest.py --port /dev/ttyUSB0 --log uart_quick.log
```

The port option can be omitted when exactly one CP210x adapter with VID:PID
`10c4:ea60` is connected.

The fast test is deliberately fixed and suitable for a future board-wide
acceptance suite. It runs the same 60 cases in polling, ISR and DMA modes at
14400, 115200 and 921600 baud: 540 hardware cases in total. The 60 cases per
mode and rate are:

- one 256-byte loopback;
- one 6144-byte loopback;
- ten 6144-byte stress loopbacks;
- 23 exact-length TX cases covering every `size % 8` value;
- 23 exact-length RX cases covering every `size % 8` value.
- one forced-timeout case followed by a real CRC loopback recovery check;
- one 6144-byte payload at cache-line offset `+1`, with guard regions checked
  before and after the frame.

For DMA RX sizes with a 1–7 byte remainder, the aligned prefix is transferred
at the TI-recommended eight-byte FIFO threshold. The exact-length remainder is
then received by polling with a bounded software timeout. This is the validated
high-baud strategy; it avoids changing RX ownership to an ISR in the middle of
one continuous frame.

The intended runtime is about 12–13 minutes on the tested setup and remains
below 15 minutes with normal host scheduling.
Actual time depends on the USB-UART adapter, host scheduling and configured
timeouts. The runner stops on the first real hardware failure, records the
failing transport/rate/section, and restores 115200 baud after every rate.

Verify framing, CRC and argument tables without hardware:

```bash
python3 tools/uart_autotest.py --self-test
```

## Extended testing

Use `uart_extended_autotest.py` when investigating a driver change, a corner
case or a particular adapter. Its internal automation protocol is intentionally
not shown in CLI help and is not intended for manual terminal use.

Run the 60-case suite for all transports at the current 115200 baud:

```bash
python3 tools/uart_extended_autotest.py --mode suite \
    --transports polling,isr,dma --port /dev/ttyUSB0 \
    --log uart_all_modes_115200.log
```

Run one transport and one focused operation:

```bash
python3 tools/uart_extended_autotest.py --mode loopback \
    --transports dma --profile stress --port /dev/ttyUSB0

python3 tools/uart_extended_autotest.py --mode tx-tail \
    --transports polling --port /dev/ttyUSB0

python3 tools/uart_extended_autotest.py --mode rx-tail \
    --transports isr --port /dev/ttyUSB0
```

Run selected rates for all transports:

```bash
python3 tools/uart_extended_autotest.py --mode baud-matrix \
    --transports polling,isr,dma --bauds 14400,115200,921600 \
    --port /dev/ttyUSB0 --log uart_selected_bauds.log
```

Run the slow legacy range only when it is specifically relevant:

```bash
python3 tools/uart_extended_autotest.py --mode baud-matrix \
    --transports dma --bauds 300,600,1200,2400,4800,9600 \
    --port /dev/ttyUSB0 --log uart_low_bauds.log
```

With no `--bauds` selection, baud-matrix covers every library enum value:
300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600,
115200, 230400, 460800, 921600, 1843200 and 3686400. The two highest rates
may be reported as `UNSUPPORTED` by CP210x adapters; that is kept separate from
a test `FAIL`. Index 13 is nominally 460800; the historical enum name
`KBPS_480_8` remains for source compatibility.

The old `uart_dma_autotest.py` filename remains as a compatibility entry point
for saved commands, but new test commands should use one of the two runners
above.

## Binary protocol

All three modes use identical deterministic data and CRC-16/CCITT-FALSE
validation. A loopback frame contains a 16-byte little-endian header, an
xorshift32 payload, zero padding to an eight-byte boundary, and a little-endian
CRC16 in the final two bytes. The maximum payload is 6144 bytes. Identical
framing makes failures comparable across polling, ISR and DMA rather than
testing three different data formats.

The generated image is `examples/uart_cli/am335x_uart_cli.elf` under the
selected CMake build directory.
