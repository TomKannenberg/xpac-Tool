cmake_minimum_required(VERSION 3.16)
project(XpacTool LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ZLIB REQUIRED)

find_package(glfw3 3.3 REQUIRED)

set(IMGUI_SOURCES
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_glfw.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(xpac-tool
    src/main.cpp
    src/gui.cpp
    xpac_unpacker.cpp
    xpac_utilities.cpp
    third_party/gl3w/src/gl3w.c
    third_party/tinyfiledialogs/tinyfiledialogs.c
    ${IMGUI_SOURCES}
)

target_include_directories(xpac-tool PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    third_party/imgui
    third_party/imgui/backends
    third_party/gl3w/include
    ${ZLIB_INCLUDE_DIRS}
    third_party/tinyfiledialogs
)

target_compile_definitions(xpac-tool PRIVATE IMGUI_IMPL_OPENGL_LOADER_GL3W)

target_link_libraries(xpac-tool PRIVATE
    glfw
    ZLIB::ZLIB
    dl
    pthread
    X11
    Xext
    Xi
    Xrandr
    Xinerama
    Xcursor
)
