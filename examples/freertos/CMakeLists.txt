set(FREERTOS_EXAMPLE_TARGET am335x_freertos)

set(FREERTOS_KERNEL_DIR "${PROJECT_SOURCE_DIR}/thirdparty/FreeRTOS-Kernel")

set(FREERTOS_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config")

set(FREERTOS_PORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/port")

# ============================================================================
# FreeRTOS Kernel
# ============================================================================

add_library(freertos_kernel STATIC
        "${FREERTOS_KERNEL_DIR}/tasks.c"
        "${FREERTOS_KERNEL_DIR}/list.c"
        "${FREERTOS_KERNEL_DIR}/queue.c"
        "${FREERTOS_KERNEL_DIR}/timers.c"
        "${FREERTOS_KERNEL_DIR}/event_groups.c"
        "${FREERTOS_KERNEL_DIR}/stream_buffer.c"
        "${FREERTOS_KERNEL_DIR}/portable/MemMang/heap_4.c"
)

target_include_directories(freertos_kernel
        PUBLIC
        "${FREERTOS_KERNEL_DIR}/include"
        "${FREERTOS_CONFIG_DIR}"
)

target_compile_options(freertos_kernel
        PUBLIC
        -mcpu=cortex-a8
        -mtune=cortex-a8
        -mfloat-abi=hard
        -mfpu=neon
        -marm
        -mthumb-interwork
        -mlong-calls
        -ffreestanding
        -fdata-sections
        -ffunction-sections
)

target_compile_definitions(freertos_kernel
        PUBLIC
        __ARM_ARCH_7A__=1
        ARM_ARCH_7A=1
        AM335X_FREERTOS=1
)

# ============================================================================
# AM335x FreeRTOS port
# ============================================================================

add_library(am335x_freertos_port STATIC
        port/port.c
        port/startup.s
        port/exceptionhandlers.s
)

target_include_directories(am335x_freertos_port
        PUBLIC
        "${FREERTOS_PORT_DIR}"
        "${FREERTOS_CONFIG_DIR}"
)

target_link_libraries(am335x_freertos_port
        PUBLIC
        freertos_kernel
        am335x::bsp
)

target_compile_options(am335x_freertos_port
        PRIVATE
        $<$<COMPILE_LANGUAGE:ASM>:
        -x
        assembler-with-cpp
        >
)

# ============================================================================
# Конечная FreeRTOS-прошивка
# ============================================================================

add_am335x_firmware(${FREERTOS_EXAMPLE_TARGET}
        LINKER_SCRIPT
        "${CMAKE_CURRENT_SOURCE_DIR}/am335x_freertos.ld"

        SOURCES
        src/main.cpp
        src/init.cpp
        src/edma_test.cpp

        LIBRARIES
        am335x_freertos_port

        INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${FREERTOS_CONFIG_DIR}"

        COMPILE_DEFINITIONS
        AM335X_FREERTOS=1

        GENERATE_HEX
        GENERATE_BIN
)

set_target_properties(${FREERTOS_EXAMPLE_TARGET}
        PROPERTIES
        SUFFIX ".elf"
)