cmake_minimum_required(VERSION 3.16)
project(GuiTest)

set(CMAKE_CXX_STANDARD 17)

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

# ImGui — все .cpp файлы из папки imgui и backends
file(GLOB IMGUI_SOURCES
    imgui/*.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
# Путь к GLFW для ImGui
target_include_directories(imgui PUBLIC
    imgui
    imgui/backends
    ${glfw_SOURCE_DIR}/include
)
find_package(OpenGL REQUIRED)
# Приложение
add_executable(gui-test gui_main.cpp)
target_link_libraries(gui-test imgui glfw OpenGL::GL)
