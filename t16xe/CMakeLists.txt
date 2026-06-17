cmake_minimum_required(VERSION 4.0)
project(t16xe)
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/kernel/t16xe.cpp
    src/kernel/execute.cpp
    src/kernel/utils.cpp
    src/asm/tasm.cpp
    src/validator/TasmValidator.cpp
)
target_link_directories(t16xe PUBLIC src)