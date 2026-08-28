set(UART_CLI_TARGET am335x_uart_cli)

add_am335x_firmware(${UART_CLI_TARGET}
        LINKER_SCRIPT
        "${CMAKE_CURRENT_SOURCE_DIR}/../common/baremetal_runtime/am335x_baremetal.ld"

        SOURCES
        src/main.cpp
        src/init.cpp
        src/line_editor.cpp

        LIBRARIES
        am335x::bsp
        am335x::baremetal_runtime

        INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}/src"

        COMPILE_DEFINITIONS
        AM335X_BAREMETAL=1
        UART_CLI_EXAMPLE=1

        GENERATE_HEX
        GENERATE_BIN
)

set_target_properties(${UART_CLI_TARGET} PROPERTIES SUFFIX ".elf")
