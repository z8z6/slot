cmake_minimum_required(VERSION 3.31)
project(slot)
set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(UNICODE _UNICODE)
add_compile_options("/utf-8")
add_definitions(-DNOMINMAX)

# CONFIGURE_DEPENDS 让新增控件/声明层源文件自动进入目标，避免只增文件后链接缺符号。
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp include/*.h third/*.h)

# 由清单在任何 HWND 创建前启用 Per-Monitor V2，避免 Windows 对交换链做低清
# 位图拉伸；CMake 会在 MSVC 链接的 manifest 合并阶段嵌入该源文件。
add_executable(slot ${SOURCE_FILES} "src/slot.manifest")

set_property(TARGET slot PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(slot PUBLIC include include/Object include/Resource third)

target_link_libraries(
        slot
        dxgi.lib
        d3d12.lib
        dxguid.lib
        d3dcompiler.lib #fxc
        dxcompiler.lib  #dxc
        d3d11.lib
        d2d1.lib
        dwrite.lib
        dwmapi.lib
        windowscodecs.lib
)

set_target_properties(slot PROPERTIES
        WIN32_EXECUTABLE TRUE
)

add_custom_target(gen-docs
        COMMAND doxygen Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building HTML documentation with Doxygen"
)

option(SLOT_BUILD_TESTS "Build Slot architecture tests" ON)
if(SLOT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# cmake .. -G "Visual Studio 18 2026"
