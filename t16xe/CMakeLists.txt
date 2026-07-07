add_library(t16xe_lib STATIC
        kernel/t16xe.cpp
        kernel/execute.cpp
        kernel/utils.cpp
        kernel/tools.hpp
        kernel/t16xe.hpp
)
target_include_directories(t16xe_lib PUBLIC kernel)

add_executable(t16xe main.cpp)
target_link_libraries(t16xe t16xe_lib)