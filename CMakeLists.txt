cmake_minimum_required(VERSION 3.16)
project(ToruProcessorSuite)

set(CMAKE_CXX_STANDARD 17)

# Сборка t16xe
add_subdirectory(t16xe)

# Сборка tcc (опционально)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tcc/CMakeLists.txt)
    add_subdirectory(tcc)
endif()

# GUI (опционально, только если OpenGL найден)
find_package(OpenGL QUIET)
if(OpenGL_FOUND)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/gui/CMakeLists.txt)
        add_subdirectory(gui)
    endif()
endif()