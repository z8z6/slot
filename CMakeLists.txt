cmake_minimum_required(VERSION 3.31)
project(slot)
set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(UNICODE _UNICODE)
add_compile_options("/utf-8")
add_definitions(-DNOMINMAX)

add_subdirectory(third/yoga)

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
        yogacore
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
    include(GoogleTest)
    file(GLOB_RECURSE SLOT_TEST_SOURCES CONFIGURE_DEPENDS tests/*.cpp)
    # UI 测试按 Declarative、Controls、Layout 分类，且不创建窗口或 D3D12 Device。
    add_executable(slot_ui_tests
            ${SLOT_TEST_SOURCES}
            src/UI/Declarative/ControlFactory.cpp
            src/UI/Declarative/ImmediateUI.cpp
            src/UI/Declarative/XamlLoader.cpp
            src/UI/Layout/BaseNode.cpp
            src/UI/Layout/Layout.cpp
            src/UI/Layout/PanelNode.cpp
            src/UI/Layout/RectNode.cpp
            src/Material/Material.cpp
            src/Material/MetalMaterial.cpp
            src/Light/Light.cpp
            src/Light/ParallelLight.cpp
            src/Mesh/CubeMesh.cpp
            src/Mesh/GridMesh.cpp
            src/Mesh/Mesh.cpp
            src/Mesh/MountainMesh.cpp
            src/Mesh/ObjMeshImporter.cpp
            src/Mesh/RectMesh.cpp
            src/Mesh/SkullMesh.cpp
            src/Mesh/SphereMesh.cpp
            src/Mesh/Vertex.cpp
            src/Object/GameObject/GameObject.cpp
            src/Object/GameObject/CubeObject.cpp
            src/Object/GameObject/SimpleGameObject.cpp
            src/Object/Camera/Camera.cpp
            src/Object/Transform.cpp
            src/Object/UIObject/UIObject.cpp
            src/Object/UIObject/RectUIObject.cpp
            src/Resource/ResourceManager.cpp
            src/Core/Scene.cpp
            src/Target/DirectX/DX12Shader.cpp
            src/Util/MeshGenerator.cpp
            "${GENERATED_SHADER_REGISTRY}")
    target_include_directories(slot_ui_tests PRIVATE include third)
    target_compile_definitions(slot_ui_tests PRIVATE
            SLOT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(slot_ui_tests PRIVATE
            d3dcompiler.lib dxcompiler.lib yogacore GTest::gtest_main)
    gtest_discover_tests(slot_ui_tests)
endif()

# cmake .. -G "Visual Studio 18 2026"
