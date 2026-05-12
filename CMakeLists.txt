cmake_minimum_required(VERSION 3.31)
project(slot)
set(CMAKE_CXX_STANDARD 17)

add_compile_definitions(UNICODE _UNICODE)
add_compile_options("/utf-8")

file(GLOB_RECURSE SOURCE_FILES src/*.cpp include/*.h third/*.h)

add_executable(slot ${SOURCE_FILES})

set_property(TARGET slot PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(slot PUBLIC include third)

target_link_libraries(
        slot
        dxgi.lib
        d3d12.lib
        dxguid.lib
        d3dcompiler.lib
)

set_target_properties(slot PROPERTIES
        WIN32_EXECUTABLE TRUE
)

# cmake .. -G "Visual Studio 18 2026"