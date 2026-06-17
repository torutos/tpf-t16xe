add_executable(tcc
    tcc.cpp
    ../t16xe/src/asm/tasm.cpp
    ../t16xe/src/kernel/t16xe.cpp
    ../t16xe/src/kernel/execute.cpp
    ../t16xe/src/kernel/utils.cpp
    ../t16xe/src/kernel/tools.hpp
)
target_link_directories(tcc PUBLIC)