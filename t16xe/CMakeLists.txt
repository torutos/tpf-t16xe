add_executable(t16xe
    src/main.cpp
    src/kernel/t16xe.cpp
    src/kernel/execute.cpp
    src/kernel/utils.cpp
    src/asm/tasm.cpp
    src/validator/TasmValidator.cpp
)

target_include_directories(t16xe PUBLIC src)