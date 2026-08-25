add_library(am335x_baremetal_runtime OBJECT
        startup.s
        exceptionhandlers.s
)

add_library(am335x::baremetal_runtime ALIAS am335x_baremetal_runtime)

target_link_libraries(am335x_baremetal_runtime
        PUBLIC
        am335x::bsp
)

target_compile_options(am335x_baremetal_runtime
        PRIVATE
        $<$<COMPILE_LANGUAGE:ASM>:
        -x
        assembler-with-cpp
        >
)