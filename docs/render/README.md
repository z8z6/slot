# D3D12 渲染后端

## 组成与初始化

DX12Render 每窗口一份，组合 Device、Command、SwapChain、RenderTarget、DepthStencil、MSAA、Mesh/Material Manager、RootSignature 和两个 RenderBatch。Device 是进程单例，Debug 构建启用 D3D12 Debug Layer。

初始化顺序：

```text
MSAA → Command → SwapChain → RTV/DSV → Resize
→ ShaderLibrary → RootSignature → Mesh → Material → 3D/UI Batch
→ 执行上传并等待 GPU
```

## 每帧

Update 依次更新 Camera、GlobalConst 和两个批次的对象常量。Draw 重置命令资源、切换后备缓冲状态、设置 viewport/scissor、清屏、绑定目标和根签名、绘制 3D/UI，再切回 Present、提交、Present 并等待 Fence。

静态顶点、索引和材质进入 DEFAULT heap，通过 UPLOAD heap 上传；每帧常量使用持久映射 UPLOAD heap。对象 CBV 以批次中最大的对象常量结构为统一步长，并按 256 字节对齐。普通物体与 UI 都以 World、WorldInvTranspose 为固定前缀，UI 再追加颜色和裁剪数据，因此两类常量可以安全共存。

## Batch 与 PSO

RenderBatch 在初始化时把对象的类型化软引用解析成资源 Handle，并保存子网格、Material、ShaderProgram、PSO 和常量索引。绘制时逐对象绑定对应的 PSO、b1 材质和 b0 对象常量，b2 保存全局常量。

批次按 ShaderProgram 缓存 PSO，Program 描述 VS/PS、深度和混合状态，不再根据首对象或 RTTI 推断管线。3D 批次按 Program、Material、Mesh 稳定排序以减少状态切换；透明 UI 批次保持声明顺序，确保画家算法的叠放语义。空批次现已支持。

每帧等待 GPU 简单安全，但无法 CPU/GPU 多帧并行。后续应引入按交换链帧数划分的 Frame Resource。Flip swap chain 也不能直接多采样，MSAA 应使用独立 RT 后 Resolve。

关键源码：`DX12Render.cpp`、`DX12RenderBatch.cpp`、`DX12Command.cpp`、`DX12PipelineState.cpp`、`DX12ConstBuffer.cpp`。
