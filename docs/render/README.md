# D3D12 渲染后端

## 组成与初始化

DX12Render 每窗口一份，组合 Device、Command、SwapChain、RenderTarget、DepthStencil、MSAA、Mesh/Material Manager、RootSignature 和两个 RenderBatch。Device 是进程单例，Debug 构建启用 D3D12 Debug Layer。

初始化顺序：

```text
MSAA → Command → SwapChain → RTV/DSV → Resize
→ RootSignature → Mesh → Material → 3D/UI Batch
→ 执行上传并等待 GPU
```

## 每帧

Update 依次更新 Camera、GlobalConst 和两个批次的对象常量。Draw 重置命令资源、切换后备缓冲状态、设置 viewport/scissor、清屏、绑定目标和根签名、绘制 3D/UI，再切回 Present、提交、Present 并等待 Fence。

静态顶点、索引和材质进入 DEFAULT heap，通过 UPLOAD heap 上传；每帧常量使用持久映射 UPLOAD heap。对象 CBV 以批次中最大的对象常量结构为统一步长，并按 256 字节对齐，因此普通世界矩阵和带颜色的 UI 常量可以安全共存。

## Batch 与 PSO

RenderBatch 保存对象、子网格和常量索引。绘制时绑定 b1 材质、b2 全局常量，并逐对象绑定 b0 和 DrawIndexedInstanced。

每批只用首个对象创建 PSO，因此批内对象必须使用兼容 Shader/状态。空批次现已支持。检测到 UIObject 时，PSO 会关闭深度测试/写入并启用 alpha blend，使控件按声明顺序叠放。

每帧等待 GPU 简单安全，但无法 CPU/GPU 多帧并行。后续应引入按交换链帧数划分的 Frame Resource。Flip swap chain 也不能直接多采样，MSAA 应使用独立 RT 后 Resolve。

关键源码：`DX12Render.cpp`、`DX12RenderBatch.cpp`、`DX12Command.cpp`、`DX12PipelineState.cpp`、`DX12ConstBuffer.cpp`。
