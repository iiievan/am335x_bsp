set(BOOTLOADER_TARGET am335x_boot)
option(AM335X_BOOT_LOG_UART "Mirror bootloader logs to polling UART0" ON)
option(AM335X_BOOT_LOG_RTT "Include RTT in bootloader (reserves 64 KiB)" OFF)

set(BOOT_LOG_LIBRARIES)
if(AM335X_BOOT_LOG_RTT)
    if(NOT TARGET am335x::log_rtt)
        message(FATAL_ERROR "AM335X_BOOT_LOG_RTT requires AM335X_BSP_ENABLE_RTT_BACKEND=ON")
    endif()
    list(APPEND BOOT_LOG_LIBRARIES am335x::log_rtt)
    set(BOOT_SRAM_LENGTH 0xFC00)
    set(BOOT_RTT_ORIGIN 0x40300000)
    set(BOOT_RTT_LENGTH 0x10000)
else()
    set(BOOT_SRAM_LENGTH 0x1FC00)
    set(BOOT_RTT_ORIGIN 0x40310000)
    set(BOOT_RTT_LENGTH 0x0)
endif()
set(BOOTLOADER_LINKER_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/am335x_boot.ld")
configure_file(am335x_boot.ld.in "${BOOTLOADER_LINKER_SCRIPT}" @ONLY)

add_am335x_firmware(${BOOTLOADER_TARGET}
        LINKER_SCRIPT
        "${BOOTLOADER_LINKER_SCRIPT}"

        SOURCES
        src/main.cpp
        src/init.cpp
        src/ddr_calibration.cpp

        LIBRARIES
        am335x::bsp
        ${BOOT_LOG_LIBRARIES}
        am335x::baremetal_runtime

        INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}/src"

        GENERATE_HEX
        GENERATE_BIN
)

# ELF будет называться am335x_boot.elf.
set_target_properties(${BOOTLOADER_TARGET} PROPERTIES SUFFIX ".elf")
target_compile_definitions(${BOOTLOADER_TARGET} PRIVATE
        AM335X_BOOT_LOG_RTT=$<BOOL:${AM335X_BOOT_LOG_RTT}>
        AM335X_BOOT_LOG_UART=$<BOOL:${AM335X_BOOT_LOG_UART}>)
if(NOT AM335X_BOOT_LOG_RTT)
    add_custom_command(TARGET ${BOOTLOADER_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
            "-DNM=${CMAKE_NM}"
            "-DELF=$<TARGET_FILE:${BOOTLOADER_TARGET}>"
            -P "${PROJECT_SOURCE_DIR}/cmake/CheckNoRtt.cmake"
            COMMENT "Checking bootloader has no SEGGER RTT symbols"
            VERBATIM)
endif()

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
