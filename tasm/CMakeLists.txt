add_library(tasm_lib STATIC
        asm/tasm.cpp
        asm/tasm.hpp
)
target_include_directories(tasm_lib PUBLIC asm)

add_executable(tasm main.cpp)
target_link_libraries(tasm tasm_lib)