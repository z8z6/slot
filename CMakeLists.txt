cmake_minimum_required(VERSION 3.31)
project(slot)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp include/*.h)

add_executable(slot ${SOURCE_FILES})

target_include_directories(slot PUBLIC include)

target_link_libraries(
        slot
        dxgi.lib
        d3d12.lib
        dxguid.lib
        d3dcompiler.lib
)

# cmake .. -G "Visual Studio 17 2022"