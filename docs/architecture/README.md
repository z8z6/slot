# 总体架构

Slot 是面向 Windows 的实时渲染框架。Win32 提供窗口和输入，Direct3D 12 是当前唯一实际后端，DirectXMath 处理变换，Yoga 计算 Flexbox UI 布局。

```mermaid
flowchart TD
    Main[WinMain] --> App[Application]
    App --> Scene[Scene 所有权]
    App --> Assets[ResourceManager]
    App --> Layout[Yoga Layout]
    App --> Camera[Camera / Light]
    App --> Render[Render 接口]
    Render --> DX[DX12Render]
    DX --> Resources[命令/交换链/RT/DS]
    DX --> Managers[Mesh/Material Manager]
    DX --> GO[3D Batch]
    DX --> UI[UI Batch]
```

## 分层

| 层 | 职责 | 目录 |
| --- | --- | --- |
| 应用平台 | 窗口、消息、计时、应用编排 | `Core` |
| 场景表示 | Scene、对象、变换、相机、灯光 | `Scene`、`Object`、`Light` |
| 资源表示 | 强类型句柄、Mesh、Material、ShaderProgram | `Resource`、`Mesh`、`Material`、`Shader` |
| UI | Yoga 节点与 UI 渲染对象 | `UI/Layout` |
| 后端抽象 | `Init/Update/Draw/Resize` | `Target/Render.*` |
| D3D12 | GPU 资源、PSO、批次和提交 | `Target/DirectX` |

`Render::CreateRender` 当前始终创建 `DX12Render`，DirectX 11 和 Vulkan 枚举仅为预留。每个窗口拥有独立渲染器与交换链，`DX12Device` 的 Factory/Device 是进程级单例。

`Application` 显式拥有 `ResourceManager` 和活动 `Scene`。ResourceManager 独占 CPU 资源，Scene 独占相机、主灯光和 GameObject；Renderer 与输入系统只保存生命周期受限的观察指针。UI Layout 是窗口级覆盖层，不因 3D Scene 切换而自动销毁。

场景使用类型化软引用表达资源依赖，批次初始化时解析为运行时 Handle。DX12 缓存只拥有设备相关数据，避免资源表示层依赖具体后端。

每帧先由 Yoga 写入 UI 像素位置，再更新相机、全局常量和对象矩阵；3D/UI 数据分别进入两个批次，最后由 D3D12 提交并 Present。
