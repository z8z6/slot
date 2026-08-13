# Slot 项目知识文档

本文档集基于当前源码整理，描述已经实现的行为；规划能力会明确标注。

## 推荐阅读顺序

1. [总体架构](architecture/README.md)：模块边界、依赖关系与数据流。
2. [应用核心](core/README.md)：启动、窗口、主循环与输入。
3. [场景对象](scene/README.md)：Object、Transform、相机和灯光。
4. [资源管理](resource/README.md)：Asset ID、强类型 Handle、所有权和后端缓存。
5. [模型与网格](model/README.md)：顶点、网格注册、合并上传和 OBJ。
6. [材质](material/README.md)：材质数据与绑定方式。
7. [着色器](shader/README.md)：ShaderProgram 生成、根签名和常量 ABI。
8. [D3D12 渲染](render/README.md)：资源、批次和逐帧绘制。
9. [UI](ui/README.md)：原生 Flex 布局和屏幕空间渲染。
10. [限制与演进](roadmap/README.md)：已知问题及推荐改造顺序。

## 模块索引

| 模块 | 关键源码 |
| --- | --- |
| [架构](architecture/README.md) | `src/main.cpp`、`include/Core`、`include/Target` |
| [核心](core/README.md) | `src/Core`、`include/Core` |
| [场景](scene/README.md) | `Scene`、`Object`、`Light`、`Phys` |
| [资源](resource/README.md) | `Resource`、`ResourceManager.cpp` |
| [模型](model/README.md) | `Mesh`、`DX12MeshManager.cpp` |
| [材质](material/README.md) | `Material`、`DX12MaterialManager.cpp` |
| [着色器](shader/README.md) | `Shader`、`shader`、`DX12Shader.cpp` |
| [渲染](render/README.md) | `Target/DirectX` |
| [UI](ui/README.md) | `UI/Layout`、`Object/UIObject` |

## 构建

项目使用 Windows、C++20、Direct3D 12、CMake、MSVC 和 Ninja。UI 布局由项目内置求解器实现。

```powershell
cmake -S . -B cmake-build-debug -G Ninja
cmake --build cmake-build-debug --target slot
```

Shader 和模型按相对路径读取，运行工作目录应为仓库根目录。
