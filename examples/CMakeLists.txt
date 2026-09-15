add_subdirectory(common/baremetal_runtime)
add_subdirectory(bootloader)
add_subdirectory(freertos)
add_subdirectory(edma_test)
add_subdirectory(uart_dma)
add_subdirectory(uart_cli)
add_subdirectory(sd_blocks)

# The FatFs example needs ChaN's FatFs R0.16 vendored under thirdparty/ff16.
# See examples/sd_fatfs/README.md; the example is skipped while the sources are absent so that a
# fresh clone still builds.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../thirdparty/ff16/source/ff.c")
    add_subdirectory(sd_fatfs)
else()
    message(STATUS "FatFs sources not found under thirdparty/ff16 - skipping the sd_fatfs example "
                   "(see examples/sd_fatfs/README.md)")
endif()