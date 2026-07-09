include(FetchContent)

FetchContent_Declare(
        glfw
        URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

add_executable(toru16 main.cpp)

target_include_directories(toru16 PRIVATE
        .
        ${CMAKE_SOURCE_DIR}/t16xe/kernel
        ${CMAKE_SOURCE_DIR}/libs/imgui
        ${CMAKE_SOURCE_DIR}/libs/imgui/backends
        ${glfw_SOURCE_DIR}/include
)

target_sources(toru16 PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/imgui/imgui.cpp
        ${CMAKE_SOURCE_DIR}/libs/imgui/imgui_draw.cpp
        ${CMAKE_SOURCE_DIR}/libs/imgui/imgui_widgets.cpp
        ${CMAKE_SOURCE_DIR}/libs/imgui/imgui_tables.cpp
        ${CMAKE_SOURCE_DIR}/libs/imgui/backends/imgui_impl_glfw.cpp
        ${CMAKE_SOURCE_DIR}/libs/imgui/backends/imgui_impl_opengl3.cpp
)

target_link_libraries(toru16 t16xe_lib glfw opengl32)