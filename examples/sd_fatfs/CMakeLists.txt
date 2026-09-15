set(SD_FATFS_TARGET am335x_sd_fatfs)

# ChaN's FatFs R0.16, vendored as shipped under thirdparty/ff16.
set(FATFS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/ff16/source")

foreach(required ff.c ff.h ffunicode.c diskio.h)
    if(NOT EXISTS "${FATFS_DIR}/${required}")
        message(FATAL_ERROR
                "thirdparty/ff16/source/${required} is missing "
                "(see examples/sd_fatfs/README.md).")
    endif()
endforeach()

add_am335x_firmware(${SD_FATFS_TARGET}
        LINKER_SCRIPT
        "${CMAKE_CURRENT_SOURCE_DIR}/../common/baremetal_runtime/am335x_baremetal.ld"

        SOURCES
        src/main.cpp
        src/init.cpp
        src/sd_fatfs.cpp
        src/diskio.cpp
        "${FATFS_DIR}/ff.c"
        "${FATFS_DIR}/ffunicode.c"

        LIBRARIES
        am335x::bsp
        am335x::log_rtt
        am335x::baremetal_runtime

        INCLUDE_DIRECTORIES
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${FATFS_DIR}"

        COMPILE_DEFINITIONS
        AM335X_BAREMETAL=1

        GENERATE_HEX
        GENERATE_BIN
)

# ff.h only pulls in its own ffconf.h when FFCONF_DEF is still undefined, so force-including the
# project's configuration first makes the vendored tree use it without being modified. A quoted
# #include "ffconf.h" resolves next to ff.h, so an include path alone would not be enough.
target_compile_options(${SD_FATFS_TARGET} PRIVATE
        "-include${CMAKE_CURRENT_SOURCE_DIR}/src/ffconf.h")

# FatFs is third-party code: do not hold it to the project's warning level.
set_source_files_properties("${FATFS_DIR}/ff.c" "${FATFS_DIR}/ffunicode.c"
        TARGET_DIRECTORY ${SD_FATFS_TARGET}
        PROPERTIES COMPILE_OPTIONS "-w")

set_target_properties(${SD_FATFS_TARGET}
        PROPERTIES
        SUFFIX ".elf"
)
