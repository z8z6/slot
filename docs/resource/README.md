# 资源管理

`Application::Resources` 是 CPU 资源的唯一所有者。`ResourceManager` 分别保存 Mesh、Material、Texture、Shader 和 ShaderProgram 的类型化 `ResourcePool<T>`。

场景数据使用 `ResourceRef<T>` 保存规范 Asset ID；GameObject 绑定 Mesh 和 Material，Material 再引用 ShaderProgram 与 Texture。`DX12RenderBatch::Init` 将依赖链解析为带 Index/Generation 的 `ResourceHandle<T>`，渲染热路径不做字符串查询。

运行时资源统一位于 `asset/`：网格、着色器、纹理和材质分别使用
`asset/mesh`、`asset/shader`、`asset/texture` 与 `asset/material`。内建资源 ID
位于 `include/Resource/BuiltinResource.h`。Mesh 和 Material 在
`ResourceManager::RegisterBuiltinResources` 显式注册。内建资源通过自身的
`GetName()` 提供规范 ID，`ResourceManager::Add` 根据 C++ 资源类自动选择类型池，
避免类型、ID 和注册函数在调用处重复声明；文件导入边界可通过
`Add(explicitId, resource)` 显式指定外部 ID。Resolve、TryGet 和类型池选择也复用
同一份类型映射，不再为每种资源维护函数特化。
普通 ShaderProgram 由
`asset/shader/*.shader.json` 描述，CMake 调用
`scripts/generate_shader_registry.py` 生成显式注册代码。文件级静态 Registry
已移除。

DX12 后端只拥有 GPU 表示：`DX12ShaderLibrary` 保存 DXIL，MeshManager 保存合并后的顶点/索引缓冲，MaterialManager 保存 256 字节对齐常量槽，TextureManager 保存纹理资源与 SRV。

新增 Mesh、Material、ShaderProgram 或 UI Node 时使用 `skills/add-slot-resource` 中的流程。
