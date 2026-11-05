cmake_minimum_required(VERSION 3.16)
project(ToruProcessorSuite)
set(CMAKE_CXX_STANDARD 17)
# Куда складывать скомпилированные exe/dll
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/lib)
# Ядро процессора
add_subdirectory(t16xe)

# Ассемблер
add_subdirectory(tasm)

# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tcc/CMakeLists.txt)
#     add_subdirectory(tcc)
# endif()

# GUI
find_package(OpenGL QUIET)
if(OpenGL_FOUND)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/gui/CMakeLists.txt)
        add_subdirectory(gui)

    endif()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/toru16/CMakeLists.txt)
        add_subdirectory(toru16)

    endif()
endif()