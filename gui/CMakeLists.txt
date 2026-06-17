cmake_minimum_required(VERSION 3.16)
project(gui)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)

# GLFW через FetchContent
include(FetchContent)
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# ImGui
file(GLOB IMGUI_SOURCES
    imgui/*.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    imgui
    imgui/backends
    ${glfw_SOURCE_DIR}/include
)

# Приложение
add_executable(gui gui_main.cpp TextEditor.cpp tinyfiledialogs.c)
target_link_libraries(gui imgui glfw OpenGL::GL t16xe_lib)
target_include_directories(gui PUBLIC ../t16xe/src)