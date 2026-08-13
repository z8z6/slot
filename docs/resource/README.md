# 资源管理

`Application::Resources` 是 CPU 资源的唯一所有者。`ResourceManager` 内部分别保存 Mesh、Material、Shader 和 ShaderProgram 的类型化 `ResourcePool<T>`，自身不是进程单例，因此测试和未来多设备上下文不共享隐式状态。

场景数据使用 `ResourceReference<T>` 保存规范 Asset ID；GameObject 绑定 Mesh 和 Material，Material 再引用 ShaderProgram。`DX12RenderBatch::Init` 将这条依赖链解析为带 Index/Generation 的 `ResourceHandle<T>`。渲染热路径只使用 Handle，避免逐帧字符串查询，Generation 也为后续卸载和热重载提供旧句柄检测边界。

运行时资源统一位于 `asset/`：网格、着色器、纹理和材质分别使用
`asset/mesh`、`asset/shader`、`asset/texture` 与 `asset/material`。内建资源 ID
位于 `include/Resource/BuiltinResource.h`。Mesh 和 Material 在
`ResourceManager::RegisterBuiltinResources` 显式注册；普通 ShaderProgram 由
`asset/shader/*.shader.json` 描述，CMake 调用
`scripts/generate_shader_registry.py` 生成显式注册代码。文件级静态 Registry
已移除。

DX12 后端只拥有 GPU 表示：`DX12ShaderLibrary` 保存 DXIL，MeshManager 保存合并后的顶点/索引缓冲及 SubMesh，MaterialManager 为每种材质建立 256 字节对齐的常量槽位。CPU Shader 不再反向保存 `DX12Shader*`。

新增 Mesh、Material、ShaderProgram 或 UI Node 时使用 `skills/add-slot-resource` 中的流程。
