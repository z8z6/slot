include(FetchContent)
include(GoogleTest)

# 测试框架由测试目录独立管理，生产目标不会因测试开关引入 GTest。
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip)
FetchContent_MakeAvailable(googletest)

# 各测试分类共享同一份无窗口运行时，避免每个可执行文件重复编译 UI、资源与场景
# 实现。CONFIGURE_DEPENDS 使新增模块实现自动进入测试运行时，避免文件改名后
# CMake 仍指向旧路径。Core 仅收集事件和场景职责，DirectX 仅收集 Shader
# 实现，因为单元测试不应将窗口、Application 或完整渲染后端带入进程。
file(GLOB_RECURSE SLOT_TEST_RUNTIME_SOURCES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/src/Object/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/Resource/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/UI/*.cpp"
        "${PROJECT_SOURCE_DIR}/src/Util/*.cpp")
file(GLOB SLOT_TEST_CORE_SOURCES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/src/Core/*Event.cpp"
        "${PROJECT_SOURCE_DIR}/src/Core/*Scene*.cpp")
file(GLOB SLOT_TEST_SHADER_SOURCES CONFIGURE_DEPENDS
        "${PROJECT_SOURCE_DIR}/src/Target/DirectX/*Shader.cpp")

add_library(slot_test_runtime STATIC
        ${SLOT_TEST_RUNTIME_SOURCES}
        ${SLOT_TEST_CORE_SOURCES}
        ${SLOT_TEST_SHADER_SOURCES})
target_include_directories(slot_test_runtime PUBLIC
        "${PROJECT_SOURCE_DIR}/include"
        "${PROJECT_SOURCE_DIR}/include/Object"
        "${PROJECT_SOURCE_DIR}/include/Resource"
        "${PROJECT_SOURCE_DIR}/third")
target_compile_definitions(slot_test_runtime PUBLIC
        SLOT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
target_link_libraries(slot_test_runtime PUBLIC
        d3dcompiler.lib
        dxcompiler.lib
        dwrite.lib
        windowscodecs.lib)

# 为一个测试目录创建独立可执行文件并注册其中的 GoogleTest 用例。只收集
# 当前叶子目录的 cpp，因此新增测试无需修改 CMake，同时不会跨目录混入其他目标。
function(slot_add_test_target target)
    file(GLOB test_sources CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    if(NOT test_sources)
        message(FATAL_ERROR "Test target ${target} has no source files")
    endif()

    add_executable(${target} ${test_sources})
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
