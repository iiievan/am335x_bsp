cmake_minimum_required(VERSION 3.24)
project(am335x_bsp VERSION 1.0.0 LANGUAGES ASM C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(AM335X_BSP_BUILD_EXAMPLES "Build AM335x BSP examples" ON)
option(BSP_USE_HARD_FLOAT "Use hardware floating point" ON)

if(NOT CMAKE_C_COMPILER MATCHES "arm-none-eabi")
    message(WARNING "Expected arm-none-eabi-gcc, found ${CMAKE_C_COMPILER}")
    message(WARNING "Please set CMAKE_TOOLCHAIN_FILE or use proper compiler")
endif()

list(APPEND CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

add_subdirectory(thirdparty/SEGGER_RTT)

set(BSP_SOURCES  src/hal/INTC.cpp
                 src/hal/BRANCH_PREDICTION.cpp
                 src/hal/MMU.cpp
                 src/hal/CACHE.cpp
                 src/hal/hwTimer.cpp
                 src/hal/pins/am335x_pins.cpp
                 src/hal/boards/beaglebone_black.cpp
                 src/rtt/rtt_log.c
                 src/rtt/rtt_log_capi.cpp
                 src/startup/cp15.s
                 src/startup/cpu.s
                 src/startup/cxx_support.cpp
                 src/hal/UART.hpp
                 src/hal/UART.cpp
                 src/hal/UartDma.hpp
                 src/hal/EDMA/EDMA.cpp
                 src/hal/MMCSD.hpp
                 src/hal/MMCSD.cpp
                 src/hal/EDMA/EDMA.hpp
                 src/hal/EDMA/ParamBuilder.hpp
                 src/hal/EDMA/DmaChannel.hpp
                 src/hal/EDMA/InterruptDispatcher.hpp
                 src/hal/EDMA/edma_interrupt_handlers.cpp
                 src/hal/EDMA/QdmaChannel.hpp
                 src/hal/EDMA/EDMA_diagnostics.cpp
                 src/hal/EDMA/EDMA_diagnostics.hpp
                 src/hal/PERF.cpp
                 src/hal/PERF.hpp
                 src/regs/CONTROL_MODULE.cpp
                 src/regs/REGS.cpp
                 src/regs/EDMA.cpp
)

add_library(am335x_bsp STATIC ${BSP_SOURCES})
add_library(am335x::bsp ALIAS am335x_bsp)

target_include_directories(am335x_bsp PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(am335x_bsp PUBLIC SEGGER_RTT)

target_compile_definitions(am335x_bsp PUBLIC
        __ARM_ARCH_7A__=1
        __ARM_ARCH_7__=1
        __CORTEX_A=1
        am335x
)

if(CMAKE_C_COMPILER MATCHES "arm-none-eabi")
    target_compile_options(am335x_bsp PUBLIC
            -mcpu=cortex-a8
            -mtune=cortex-a8
            -mlong-calls
            -ffreestanding
            -fdata-sections
            -ffunction-sections
            -funsigned-char
    )

if(BSP_USE_HARD_FLOAT)
    target_compile_options(am335x_bsp
            PUBLIC
            -mfloat-abi=hard
            -mfpu=neon
    )
else()
    target_compile_options(am335x_bsp
            PUBLIC
            -mfloat-abi=soft
    )

    # Предупреждения относятся только к самой библиотеке.
    target_compile_options(am335x_bsp
            PRIVATE
            -Wall
            -Wextra
    )
endif()
else()
    message(WARNING "Skipping ARM-specific flags for ${CMAKE_C_COMPILER}")
endif()


target_compile_options(am335x_bsp PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions -fno-rtti -fno-use-cxa-atexit>
)

target_compile_options(am335x_bsp PRIVATE
        $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp>
)

target_compile_options(am335x_bsp
        PRIVATE
        $<$<CONFIG:Debug>:-O0;-g3>
        $<$<CONFIG:Release>:-Os;-g0>
        $<$<CONFIG:RelWithDebInfo>:-Os;-g3>
)

target_compile_definitions(am335x_bsp
        PUBLIC
        $<$<CONFIG:Debug>:
        DEBUG;
        BSP_VOLATILE_OVERLOADS=0;
        BSP_BUILD_TYPE=\"Debug\"
        >

        $<$<CONFIG:Release>:
        NDEBUG;
        BSP_VOLATILE_OVERLOADS=1;
        BSP_BUILD_TYPE=\"Release\"
        >

        $<$<CONFIG:RelWithDebInfo>:
        NDEBUG;
        BSP_VOLATILE_OVERLOADS=1;
        BSP_BUILD_TYPE=\"RelWithDebInfo\"
        >
)

if(AM335X_BSP_BUILD_EXAMPLES)
    include(AddAm335xFirmware)
    add_subdirectory(examples)
endif()

message(STATUS "Building am335x_bsp version ${PROJECT_VERSION}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  CXX Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "  Include: ${CMAKE_CURRENT_SOURCE_DIR}/src")
message(STATUS "  Build examples: ${AM335X_BSP_BUILD_EXAMPLES}")

