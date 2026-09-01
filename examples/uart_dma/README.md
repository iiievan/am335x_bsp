# UART0 EDMA echo

This bare-metal example configures UART0 in DMA mode, transmits the welcome
message, receives exactly eight bytes through EDMA channel 27, and echoes them
through EDMA channel 26.

The RX buffer is cache-line aligned. `HAL::UART::uart0_t` performs the required
D-cache maintenance before TX, before RX, and after RX completion. Complete
eight-byte chunks use EDMA; a 1–7 byte remainder is handled by bounded polling
for RX and by polling for TX.

The example uses the unified API directly: `uart.init_dma()`, `uart.read()` and
`uart.write()`. The UART object owns and releases channels 26 and 27.

The generated image is `examples/uart_dma/am335x_uart_dma.elf` under the selected
CMake build directory.
