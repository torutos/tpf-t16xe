# Статическая библиотека для GUI
add_library(t16xe_lib STATIC
    src/kernel/t16xe.cpp
    src/kernel/execute.cpp
    src/kernel/utils.cpp
    src/asm/tasm.cpp
    src/validator/TasmValidator.cpp
)

target_include_directories(t16xe_lib PUBLIC src)

# Исполняемый файл (как раньше)
add_executable(t16xe src/main.cpp)
target_link_libraries(t16xe t16xe_lib)