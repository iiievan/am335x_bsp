set(BOOTLOADER_TARGET am335x_boot)

set(BOOTLOADER_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/am335x_boot.ld")

add_am335x_firmware(${BOOTLOADER_TARGET}
        LINKER_SCRIPT
        "${BOOTLOADER_LINKER_SCRIPT}"

        SOURCES
        src/main.cpp
        src/init.cpp
        src/ddr_calibration.cpp

        LIBRARIES
        am335x::bsp
        am335x::log_rtt
        am335x::baremetal_runtime

        INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}/src"

        GENERATE_HEX
        GENERATE_BIN
)

# ELF будет называться am335x_boot.elf.
set_target_properties(${BOOTLOADER_TARGET} PROPERTIES SUFFIX ".elf")

# ============================================================================
# Генерация MLO
# ============================================================================

set(GEN_MLO_SOURCE "${PROJECT_SOURCE_DIR}/tools/gen_mlo/gen_mlo.c")

# gen_mlo запускается на компьютере разработчика, поэтому он должен
# собираться host-компилятором, а не arm-none-eabi-gcc.
#
# Пока сохраняем ваш рабочий способ через системный gcc.
set(GEN_MLO_EXECUTABLE "${CMAKE_CURRENT_BINARY_DIR}/gen_mlo")

add_custom_command(TARGET ${BOOTLOADER_TARGET}
                   POST_BUILD

                   COMMAND
                   gcc
                   -O2
                   -Wall
                   -o "${GEN_MLO_EXECUTABLE}"
                   "${GEN_MLO_SOURCE}"

                   COMMAND
                   "${GEN_MLO_EXECUTABLE}"
                   "${CMAKE_CURRENT_BINARY_DIR}/${BOOTLOADER_TARGET}.bin"
                   "${CMAKE_CURRENT_BINARY_DIR}/MLO"

                   COMMENT "Generating AM335x MLO"
                   VERBATIM
)
