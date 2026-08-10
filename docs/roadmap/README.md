# 限制与演进建议

## 已知限制

1. 一个批次只有首对象对应的 PSO，不能混合 Shader。
2. 材质未按对象绑定，HLSL 仍使用硬编码材质。
3. 纹理、SRV 和 sampler 尚未接入。
4. UI 已有 alpha 和深度策略，但仍缺少 scissor、明确 z-order 和圆角。
5. 3D 对象/网格动态变化仍不会自动重建对应批次和 GPU 缓冲。
6. 单 allocator 且每帧等待 Fence，CPU/GPU 不能多帧并行。
7. Flip 交换链的 MSAA 路径需要独立 RT 与 Resolve。
8. 跨翻译单元静态注册存在初始化顺序风险。
9. 场景层仍有裸指针资源，缺少统一所有权和释放策略。
10. OBJ 忽略 UV/法线/材质且仅支持 16 位索引。

## 推荐顺序

1. 按 Shader/PSO、Material、RenderState 对批次分组。
2. 引入 Frame Resource、逐帧 allocator/常量区/fence。
3. 完成材质 ABI，再增加纹理 SRV 和 sampler。
4. 建立 UI 专用 PSO、alpha、scissor 与 z-order。
5. 继续将场景层裸指针迁移到 RAII；UI 控件树和 Shader 注册表已明确所有权。
6. 扩展模型导入，再完善 UI 控件、事件和物理。

修改 CPU/HLSL 共享结构时必须核对字段顺序、16 字节打包及 CBV 256 字节对齐；修改矩阵时必须同时验证转置和 `mul` 顺序；修改资源状态时应使用 D3D12 Debug Layer 验证。
