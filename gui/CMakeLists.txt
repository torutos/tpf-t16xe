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
add_executable(tpsgui
        main.cpp
        app.cpp
        panels/editor_panel.cpp
        panels/emulator_panel.cpp
        panels/registers_panel.cpp
        panels/memory_panel.cpp
        ../libs/TextEditor.cpp
        ../libs/tinyfiledialogs.c
        ../libs/imgui/imgui.cpp
        ../libs/imgui/imgui_draw.cpp
        ../libs/imgui/imgui_widgets.cpp
        ../libs/imgui/imgui_tables.cpp
        ../libs/imgui/backends/imgui_impl_glfw.cpp
        ../libs/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(tpsgui PRIVATE
        ../libs
        ../libs/imgui
        ${CMAKE_SOURCE_DIR}/t16xe/
        ${CMAKE_SOURCE_DIR}/tasm/
        ${glfw_SOURCE_DIR}/include
)

target_link_libraries(tpsgui
        t16xe_lib
        tasm_lib
        glfw
        opengl32
)