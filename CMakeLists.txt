cmake_minimum_required(VERSION 3.31)
project(slot)
set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(UNICODE _UNICODE)
add_compile_options("/utf-8")
add_definitions(-DNOMINMAX)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

# 根据 Shader 描述文件生成相应的 C++ 注册代码
file(GLOB SHADER_MANIFESTS CONFIGURE_DEPENDS shader/*.shader.json)
set(GENERATED_SHADER_REGISTRY
        "${CMAKE_CURRENT_BINARY_DIR}/generated/ShaderRegistry.generated.cpp")
add_custom_command(
        OUTPUT "${GENERATED_SHADER_REGISTRY}"
        COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_shader_registry.py"
                --output "${GENERATED_SHADER_REGISTRY}"
                ${SHADER_MANIFESTS}
        DEPENDS ${SHADER_MANIFESTS}
                "${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_shader_registry.py"
        COMMENT "Generating ShaderProgram registry"
        VERBATIM)

# CONFIGURE_DEPENDS 让新增控件/声明层源文件自动进入目标，避免只增文件后链接缺符号。
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp include/*.h third/*.h)

add_executable(slot ${SOURCE_FILES} "${GENERATED_SHADER_REGISTRY}")

set_property(TARGET slot PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(slot PUBLIC include third)

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
