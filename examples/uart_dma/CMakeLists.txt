set(UART_DMA_TARGET am335x_uart_dma)

add_am335x_firmware(${UART_DMA_TARGET}
        LINKER_SCRIPT
        "${CMAKE_CURRENT_SOURCE_DIR}/../edma_test/am335x_edma_test.ld"

        SOURCES
        src/main.cpp
        src/init.cpp

        LIBRARIES
        am335x::bsp
        am335x::baremetal_runtime

        INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}/src"

        COMPILE_DEFINITIONS
        AM335X_BAREMETAL=1
        UART_DMA_EXAMPLE=1

        GENERATE_HEX
        GENERATE_BIN
)

set_target_properties(${UART_DMA_TARGET} PROPERTIES SUFFIX ".elf")
