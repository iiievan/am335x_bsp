# UART0 CLI tests

This bare-metal example keeps its command-line interface in UART0 polling mode
and provides these commands:

- `help`
- `test polling`
- `test interrupt`
- `test dma`
- `test all`

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

The generated image is `examples/uart_cli/am335x_uart_cli.elf` under the
selected CMake build directory.
