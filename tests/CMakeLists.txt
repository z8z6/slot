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
        ../src/Core/SceneObjectDetailsBinding.cpp
        ../src/Core/ScenePicker.cpp
        ../src/UI/Dock/DockTree.cpp
        ../src/UI/Dock/DockWorkspace.cpp
        ../src/UI/Behavior/DockBehavior.cpp
        ../src/UI/Behavior/DragBehavior.cpp
        ../src/UI/Behavior/ResizeBehavior.cpp
        ../src/UI/Behavior/ScrollBehavior.cpp
        ../src/UI/Declarative/ControlFactory.cpp
        ../src/UI/Declarative/ImmediateUI.cpp
        ../src/UI/Declarative/XamlHotReload.cpp
        ../src/UI/Declarative/XamlLoader.cpp
        ../src/UI/Layout/BaseNode.cpp
        ../src/UI/Layout/BehaviorNode.cpp
        ../src/UI/Layout/ButtonNode.cpp
        ../src/UI/Layout/DrawNode.cpp
        ../src/UI/Layout/FileExplorerNode.cpp
        ../src/UI/Layout/Layout.cpp
        ../src/UI/Layout/LayoutEngine.cpp
        ../src/UI/Layout/ImageNode.cpp
        ../src/UI/Layout/MenuNode.cpp
        ../src/UI/Layout/PanelGroupNode.cpp
        ../src/UI/Layout/PanelNode.cpp
        ../src/UI/Layout/RectNode.cpp
        ../src/UI/Layout/SceneNode.cpp
        ../src/UI/Layout/SliderNode.cpp
        ../src/UI/Layout/ScrollBarNode.cpp
        ../src/UI/Layout/ScrollNode.cpp
        ../src/UI/Layout/TerminalNode.cpp
        ../src/UI/Layout/TextNode.cpp
        ../src/UI/Layout/TextInputNode.cpp
        ../src/UI/Layout/ToggleNode.cpp
        ../src/UI/Layout/ToolBarNode.cpp
        ../src/UI/Layout/TreeViewNode.cpp
        ../src/UI/Style/Theme.cpp
        ../src/Material/Material.cpp
        ../src/Material/BuiltinMaterial.cpp
        ../src/Light/BaseLight.cpp
        ../src/Light/ParallelLight.cpp
        ../src/Mesh/CubeMesh.cpp
        ../src/Mesh/FbxMeshImporter.cpp
        ../src/Mesh/GridMesh.cpp
        ../src/Mesh/Mesh.cpp
        ../src/Mesh/MountainMesh.cpp
        ../src/Mesh/ObjMeshImporter.cpp
        ../src/Mesh/RectMesh.cpp
        ../src/Mesh/SkullMesh.cpp
        ../src/Mesh/SphereMesh.cpp
        ../src/Mesh/TriangleMesh.cpp
        ../src/Mesh/Vertex.cpp
        ../src/Object/Camera/BaseCamera.cpp
        ../src/Object/Camera/FirstPersonCamera.cpp
        ../src/Object/GameObject/CubeObject.cpp
        ../src/Object/GameObject/GameObject.cpp
        ../src/Object/GameObject/SimpleGameObject.cpp
        ../src/Object/Transform.cpp
        ../src/Object/UIObject/RectUIObject.cpp
        ../src/Object/UIObject/UIObject.cpp
        ../src/Resource/ResourceHandle.cpp
        ../src/Resource/ResourceManager.cpp
        ../src/Core/Scene.cpp
        ../src/Target/DirectX/DX12Shader.cpp
        ../src/Texture/Texture.cpp
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
        dwrite.lib
        windowscodecs.lib)

# 为一个测试目录创建独立可执行文件并注册其中的 GoogleTest 用例。叶子目录
# 显式传入源文件，避免递归 glob 让新增目录意外混入其他测试目标。
function(slot_add_test_target target)
    add_executable(${target} ${ARGN})
    target_link_libraries(${target} PRIVATE slot_test_runtime GTest::gtest_main)
    # CMake 4.3 的 GoogleTest 发现阶段会在工作目录保留 JSON 元数据，因此放在
    # 构建树而不是污染源码根。需要 asset/* 的测试使用 SLOT_SOURCE_DIR 显式
    # 建立资源根，避免把发现实现细节变成运行时路径约定。
    gtest_discover_tests(${target}
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endfunction()

add_subdirectory(Object)
add_subdirectory(Mesh)
add_subdirectory(Resource)
add_subdirectory(Scene)
add_subdirectory(Shader)
add_subdirectory(UI)

# 聚合目标只负责便捷构建，不重新合并测试进程；各分类仍可独立链接和执行。
add_custom_target(slot_tests DEPENDS
        slot_mesh_tests
        slot_object_tests
        slot_resource_tests
        slot_scene_tests
        slot_shader_tests
        slot_ui_behavior_tests
        slot_ui_controls_tests
        slot_ui_declarative_tests
        slot_ui_layout_tests
        slot_ui_style_tests)
