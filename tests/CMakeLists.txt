include(FetchContent)
include(GoogleTest)

# 测试框架由测试目录独立管理，生产目标不会因测试开关引入 GTest。
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip)
FetchContent_MakeAvailable(googletest)

# 各测试分类共享同一份无窗口运行时，避免每个可执行文件重复编译 UI、资源与场景
# 实现。该库刻意不包含 Application/Window 和完整 DX12 后端，保证单元测试不会
# 创建 HWND 或图形设备。
add_library(slot_test_runtime STATIC
        ../src/Core/Event.cpp
        ../src/UI/Dock/DockTree.cpp
        ../src/UI/Dock/DockWorkspace.cpp
        ../src/UI/Behavior/DockBehavior.cpp
        ../src/UI/Behavior/DragBehavior.cpp
        ../src/UI/Behavior/ResizeBehavior.cpp
        ../src/UI/Behavior/ScrollBehavior.cpp
        ../src/UI/Declarative/ControlFactory.cpp
        ../src/UI/Declarative/ImmediateUI.cpp
        ../src/UI/Declarative/XamlLoader.cpp
        ../src/UI/Layout/BaseNode.cpp
        ../src/UI/Layout/BehaviorNode.cpp
        ../src/UI/Layout/DrawNode.cpp
        ../src/UI/Layout/Layout.cpp
        ../src/UI/Layout/LayoutEngine.cpp
        ../src/UI/Layout/ImageNode.cpp
        ../src/UI/Layout/PanelGroupNode.cpp
        ../src/UI/Layout/PanelNode.cpp
        ../src/UI/Layout/RectNode.cpp
        ../src/UI/Layout/SceneNode.cpp
        ../src/UI/Layout/ScrollBarNode.cpp
        ../src/UI/Layout/ScrollNode.cpp
        ../src/UI/Layout/TerminalNode.cpp
        ../src/UI/Layout/TextNode.cpp
        ../src/UI/Style/Theme.cpp
        ../src/Material/Material.cpp
        ../src/Material/MetalMaterial.cpp
        ../src/Light/Light.cpp
        ../src/Light/ParallelLight.cpp
        ../src/Mesh/CubeMesh.cpp
        ../src/Mesh/GridMesh.cpp
        ../src/Mesh/Mesh.cpp
        ../src/Mesh/MountainMesh.cpp
        ../src/Mesh/ObjMeshImporter.cpp
        ../src/Mesh/RectMesh.cpp
        ../src/Mesh/SkullMesh.cpp
        ../src/Mesh/SphereMesh.cpp
        ../src/Mesh/Vertex.cpp
        ../src/Object/Camera/Camera.cpp
        ../src/Object/Camera/FirstPersonCamera.cpp
        ../src/Object/GameObject/CubeObject.cpp
        ../src/Object/GameObject/GameObject.cpp
        ../src/Object/GameObject/SimpleGameObject.cpp
        ../src/Object/Transform.cpp
        ../src/Object/UIObject/RectUIObject.cpp
        ../src/Object/UIObject/UIObject.cpp
        ../src/Resource/ResourceManager.cpp
        ../src/Core/Scene.cpp
        ../src/Target/DirectX/DX12Shader.cpp
        ../src/Util/Color.cpp
        ../src/Util/MeshGenerator.cpp
        "${GENERATED_SHADER_REGISTRY}")
target_include_directories(slot_test_runtime PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/third")
target_compile_definitions(slot_test_runtime PUBLIC
        SLOT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
target_link_libraries(slot_test_runtime PUBLIC
        d3dcompiler.lib
        dxcompiler.lib
        dwrite.lib)

# 为一个测试目录创建独立可执行文件并注册其中的 GoogleTest 用例。叶子目录
# 显式传入源文件，避免递归 glob 让新增目录意外混入其他测试目标。
function(slot_add_test_target target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE slot_test_runtime GTest::gtest_main)
    # 资源 URI 相对于项目根解析；每个独立测试目标使用同一运行目录，避免
    # CTest 从构建目录启动时把 asset/* 误判为缺失。
    gtest_discover_tests(${target} WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
endfunction()

add_subdirectory(Object)
add_subdirectory(Resource)
add_subdirectory(Scene)
add_subdirectory(Shader)
add_subdirectory(UI)

# 聚合目标只负责便捷构建，不重新合并测试进程；各分类仍可独立链接和执行。
add_custom_target(slot_tests DEPENDS
        slot_object_tests
        slot_resource_tests
        slot_scene_tests
        slot_shader_tests
        slot_ui_behavior_tests
        slot_ui_controls_tests
        slot_ui_declarative_tests
        slot_ui_layout_tests
        slot_ui_style_tests)
