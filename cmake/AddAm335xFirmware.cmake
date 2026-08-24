include_guard(GLOBAL)

include(CMakeParseArguments)

function(add_am335x_firmware TARGET)
    set(options GENERATE_HEX GENERATE_BIN)

    set(oneValueArgs LINKER_SCRIPT)

    set(multiValueArgs
            SOURCES
            LIBRARIES
            INCLUDE_DIRECTORIES
            COMPILE_DEFINITIONS)

    cmake_parse_arguments(FW
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN}
    )

    if(NOT FW_LINKER_SCRIPT)
        message(FATAL_ERROR "${TARGET}: LINKER_SCRIPT is required")
    endif()

    if(NOT EXISTS "${FW_LINKER_SCRIPT}")
        message(FATAL_ERROR "${TARGET}: linker script does not exist: "
                            "${FW_LINKER_SCRIPT}")
    endif()

    if(NOT FW_SOURCES)
        message(FATAL_ERROR "${TARGET}: SOURCES list is empty"
        )
    endif()

    add_executable(${TARGET} ${FW_SOURCES})

    target_link_libraries(${TARGET} PRIVATE ${FW_LIBRARIES})

    target_include_directories(${TARGET} PRIVATE ${FW_INCLUDE_DIRECTORIES})

    target_compile_definitions(${TARGET} PRIVATE ${FW_COMPILE_DEFINITIONS})

    target_compile_options(${TARGET}
            PRIVATE
            -Wall
            -Wextra

            $<$<COMPILE_LANGUAGE:CXX>:
            -fno-exceptions
            -fno-rtti
            -fno-use-cxa-atexit
            >

            $<$<COMPILE_LANGUAGE:ASM>:
            -x
            assembler-with-cpp
            >

            $<$<CONFIG:Debug>:-O0;-g3>
            $<$<CONFIG:Release>:-Os;-g0>
            $<$<CONFIG:RelWithDebInfo>:-Os;-g3>
    )

    set(MAP_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.map")

    target_link_options(${TARGET} PRIVATE
            -nostartfiles
            "-T${FW_LINKER_SCRIPT}"
            -Wl,-gc-sections
            -Wl,--print-memory-usage
            "-Wl,-Map,${MAP_FILE}"
            -Wl,--no-warn-rwx-segments
            -Wl,-e,Entry
            -Wl,-u,Entry
            -Wl,-u,__aeabi_uidiv
            -Wl,-u,__aeabi_idiv
            -lc
            -lgcc
    )

    # Пересобрать ELF при изменении linker script.
    set_property(TARGET ${TARGET} APPEND
                 PROPERTY LINK_DEPENDS "${FW_LINKER_SCRIPT}")

    if(FW_GENERATE_HEX)
        set(HEX_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.hex")

        add_custom_command(TARGET ${TARGET}
                           POST_BUILD
                           COMMAND
                           ${CMAKE_OBJCOPY}
                           -O ihex
                           $<TARGET_FILE:${TARGET}>
                           "${HEX_FILE}"
                           COMMENT "Generating ${TARGET}.hex"
                           VERBATIM)
    endif()

    if(FW_GENERATE_BIN)
        set(BIN_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.bin")

        add_custom_command(TARGET ${TARGET}
                           POST_BUILD
                           COMMAND
                           ${CMAKE_OBJCOPY}
                           -O binary
                           $<TARGET_FILE:${TARGET}>
                           "${BIN_FILE}"
                           COMMENT "Generating ${TARGET}.bin"
                           VERBATIM)
    endif()

    if(CMAKE_SIZE)
        add_custom_command(TARGET ${TARGET}
                           POST_BUILD
                           COMMAND
                           ${CMAKE_SIZE}
                           $<TARGET_FILE:${TARGET}>
                           COMMENT "Memory usage for ${TARGET}"
                           VERBATIM)
    endif()
endfunction()