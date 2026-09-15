set(SD_BLOCKS_TARGET am335x_sd_blocks)

add_am335x_firmware(${SD_BLOCKS_TARGET}
        LINKER_SCRIPT
        "${CMAKE_CURRENT_SOURCE_DIR}/../common/baremetal_runtime/am335x_baremetal.ld"

        SOURCES
        src/main.cpp
        src/init.cpp
        src/sd_blocks.cpp

        LIBRARIES
        am335x::bsp
        am335x::log_rtt
        am335x::baremetal_runtime

        INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}/src"

        COMPILE_DEFINITIONS
        AM335X_BAREMETAL=1

        GENERATE_HEX
        GENERATE_BIN
)

set_target_properties(${SD_BLOCKS_TARGET}
        PROPERTIES
        SUFFIX ".elf"
)
