# UART0 EDMA echo

This bare-metal example configures UART0 in DMA mode, transmits the welcome
message, receives exactly eight bytes through EDMA channel 27, and echoes them
through EDMA channel 26.

The RX buffer is cache-line aligned. `HAL::UART::Uart0Dma` performs the required
D-cache maintenance before TX, before RX, and after RX completion. RX lengths
must be a multiple of the configured FIFO trigger level (eight bytes here).

The generated image is `examples/uart_dma/am335x_uart_dma.elf` under the selected
CMake build directory.
