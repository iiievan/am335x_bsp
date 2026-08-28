# UART0 CLI tests

This bare-metal example keeps its command-line interface in UART0 polling mode
and provides these commands:

- `help`
- `test polling`
- `test interrupt`
- `test dma`
- `test all`

Each test asks for an eight-character token and echoes the received data. Do
not press Enter after a test token. The interrupt test checks RX through the
UART ISR and TX through polling. The DMA test uses `HAL::UART::Uart0Dma` and
EDMA channels 26 and 27.

After every test, including an initialization failure or timeout, UART0 is
returned to polling mode before the CLI prints the result.

The generated image is `examples/uart_cli/am335x_uart_cli.elf` under the
selected CMake build directory.
