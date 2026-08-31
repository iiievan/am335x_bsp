# UART0 CLI tests

This bare-metal example keeps its command-line interface in UART0 polling mode
and provides these commands:

- `help`
- `test polling`
- `test interrupt`
- `test dma`
- `test all`
- `auto dma <payload-size> <sequence> <seed>`
- `auto tx <transfer-size> <sequence> <seed>`
- `auto rx <transfer-size> <sequence> <seed>`

Each test asks for an eight-character token and echoes the received data. The
interrupt test checks RX through the UART ISR and TX through polling. The DMA
test uses `HAL::UART::Uart0Dma` and EDMA channels 26 and 27.

The CLI has a fixed eight-entry command history and does not use dynamic
allocation. Up/Down browse history; Left/Right, Home, End, Backspace and Delete
edit the current line. Ctrl+C cancels it and Ctrl+L clears and redraws the
screen. Tab completes a unique command, extends multiple matches to their common
prefix, or prints the matching commands when no further common extension exists.

Run `run_tio.sh` to map Enter to one LF byte and Delete to Backspace. If tio is
connected after the firmware has already printed its initial prompt, restart
the target or press Enter to display a fresh prompt.

After every test, including an initialization failure or timeout, UART0 is
returned to polling mode before the CLI prints the result.

## Automated DMA loopback

`auto dma` receives one binary frame through UART0 RX EDMA, validates its
CRC-16/CCITT-FALSE and deterministic xorshift32 payload, and echoes the exact
received frame through UART0 TX EDMA. The maximum payload is 6144 bytes. The
wire frame contains a 16-byte little-endian header, payload, zero padding to an
eight-byte EDMA boundary, and a little-endian CRC16 in the final two bytes.

Install the host dependency and run the first smoke test at 115200 baud:

```bash
python3 -m pip install -r tools/requirements.txt
python3 tools/uart_dma_autotest.py --profile smoke --log uart_dma_smoke.log
```

The script finds a CP210x adapter by VID:PID `10c4:ea60`; use
`--port /dev/ttyUSB0` to select a port explicitly. Current profiles are
`smoke` (256 bytes once), `stress` (6144 bytes ten times), and `full`
(6144 bytes one hundred times). Baud-rate matrix testing will be added after
the fixed-rate DMA path has been verified on hardware.

## Automated TX-tail matrix

`auto tx` generates an exact-size xorshift32 packet on the target, appends
CRC-16/CCITT-FALSE, and sends it with `UartDma::transmit()`. The host matrix
checks sizes `2..16` and `6144..6151`, covering pure polling, pure EDMA, and
all possible `size % 8` polling tails after a large EDMA prefix:

```bash
python3 tools/uart_dma_autotest.py --mode tx-tail --profile smoke \
    --port /dev/ttyUSB0 --log uart_tx_tail.log
```

The generated image is `examples/uart_cli/am335x_uart_cli.elf` under the
selected CMake build directory.

## Automated RX-tail matrix

`auto rx` receives the aligned prefix through UART RX EDMA and then reads the
remaining `size % 8` bytes from the RX FIFO by polling with a separate timeout.
The target independently verifies the xorshift32 data and CRC. The same sizes
as the TX-tail matrix cover polling-only packets, exact EDMA boundaries, and
all tails from one through seven bytes:

```bash
python3 tools/uart_dma_autotest.py --mode rx-tail --profile smoke \
    --port /dev/ttyUSB0 --log uart_rx_tail.log --verbose
```

## Complete regression suite

The `suite` mode runs every currently verified hardware test in one serial
session, excluding the long `full` profile: smoke loopback, one 6144-byte
loopback, ten 6144-byte stress cycles, the complete TX-tail matrix, and the
complete RX-tail matrix. The run contains 58 hardware cases:

```bash
python3 tools/uart_dma_autotest.py --mode suite \
    --port /dev/ttyUSB0 --log uart_dma_suite.log --verbose
```

Each group prints its own `SECTION` result followed by the aggregate `SUITE`
and `SUMMARY` results. The run stops at the first failure and records the
section, case, size, and tail that failed.
