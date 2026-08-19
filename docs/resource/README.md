# 资源管理

`Application::Resources` 是 CPU 资源的唯一所有者。`ResourceManager` 分别保存 Mesh、Material、Texture、Shader 和 ShaderProgram 的类型化 `ResourcePool<T>`。

`ResourceRef<T>` 统一表示资源引用：场景数据以规范 Asset ID 保存待解析引用；
GameObject 绑定 Mesh 和 Material，Material 再引用 ShaderProgram 与 Texture。
`DX12RenderBatch::Init` 将依赖链解析为只携带稳定 Index 的 `ResourceRef<T>`，
渲染热路径和 GPU 缓存不做字符串查询。资源池在 Manager 生命周期内只追加且不
复用槽位，因此无需额外的版本字段。

运行时资源统一位于 `asset/`：网格、着色器、纹理和材质分别使用
`asset/mesh`、`asset/shader`、`asset/texture` 与 `asset/material`。内建资源 ID
位于 `include/Resource/BuiltinResource.h`。Mesh 和 Material 在
`ResourceManager::RegisterBuiltinResources` 显式注册。具体资源派生类在构造时设置
`Resource::Id` 和 `Resource::Type`，`ResourceManager::Add` 将 Mesh、Material、
Texture、Shader 与 ShaderProgram 派生类归一到对应基类资源池，
避免类型、ID 和注册函数在调用处重复声明；文件导入边界可通过
`Add(explicitId, resource)` 显式指定外部 ID。Resolve、TryGet 和类型池选择也复用
同一份类型映射，不再为每种资源维护函数特化。
普通 ShaderProgram 及其阶段由 `BuiltinShader` 中的具体派生类构造，
所有 ID 均复用 `BuiltinResource.h` 中的 builtin 字符串；注册顺序先于
Program 固化阶段索引引用，资源身份与管线状态不再散落在 Manager 中。内建纹理使用具体派生类，例如 `GrassBlockTexture`
同时固化资源 ID 和文件来源。文件级静态 Registry 已移除。

DX12 后端只拥有 GPU 表示：`DX12ShaderLibrary` 保存 DXIL，MeshManager 保存合并后的顶点/索引缓冲，MaterialManager 保存 256 字节对齐常量槽，TextureManager 保存纹理资源与 SRV。

新增 Mesh、Material、ShaderProgram 或 UI Node 时使用 `skills/add-slot-resource` 中的流程。
