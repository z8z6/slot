# D3D12 渲染后端

## 组成与初始化

DX12Render 每窗口一份，组合 Device、Command、SwapChain、RenderTarget、DepthStencil、MSAA、Mesh/Material/Texture Manager、RootSignature 和两个 RenderBatch。

初始化顺序：

```text
MSAA → Command → SwapChain → RTV/DSV → Resize
→ ShaderLibrary → RootSignature → Mesh → Material → Texture → 3D/UI Batch
→ 执行上传并等待 GPU
```

## 每帧

Update 依次更新 Camera、GlobalConst 和两个批次的对象常量。Draw 重置命令资源、切换后备缓冲状态、设置 viewport/scissor、清屏、绑定目标和根签名、绘制 3D/UI，再切回 Present、提交、Present 并等待 Fence。

静态顶点、索引、材质和纹理进入 DEFAULT heap，通过 UPLOAD heap 上传；每帧常量使用持久映射 UPLOAD heap。b0/b1/b2 使用根 CBV，把唯一 Shader-visible CBV/SRV/UAV 堆留给纹理 SRV。

## Batch 与 PSO

RenderBatch 在初始化时将 Mesh、Material、ShaderProgram 与 Texture 解析为索引引用。绘制时绑定 PSO、b0 对象、b1 材质、b2 全局常量以及可选 t0 基础色纹理。

批次按 ShaderProgram 缓存 PSO，Program 描述 VS/PS、深度和混合状态，不再根据首对象或 RTTI 推断管线。3D 批次按 Program、Material、Mesh 稳定排序以减少状态切换；透明 UI 批次保持声明顺序，确保画家算法的叠放语义。空批次现已支持。

每帧等待 GPU 简单安全，但无法 CPU/GPU 多帧并行。后续应引入按交换链帧数划分的 Frame Resource。Flip swap chain 也不能直接多采样，MSAA 应使用独立 RT 后 Resolve。

关键源码：`DX12Render.cpp`、`DX12RenderBatch.cpp`、`DX12Command.cpp`、`DX12PipelineState.cpp`、`DX12ConstBuffer.cpp`。
