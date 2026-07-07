cmake_minimum_required(VERSION 4.0)
project(tcc)
add_executable(tcc
    tcc.cpp
        ../tasm/asm/tasm.cpp
        ../t16xe/kernel/t16xe.cpp
        ../t16xe/kernel/execute.cpp
        ../t16xe/kernel/utils.cpp
        ../t16xe/kernel/tools.hpp
)