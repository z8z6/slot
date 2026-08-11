# 材质

`Material` 包含 Albedo、FresnelR0、Rough 和 Name。MetalMaterial 是当前内建材质，GameObject 通过类型化软引用共享它。

`DX12Material` 保持与 HLSL `cbMaterial` 对应的字段布局。MaterialManager 上传 ResourceManager 中的全部材质，每个材质占用 256 字节对齐槽位；RenderItem 绘制前将对应 GPU 地址绑定到根参数 1 / HLSL `b1`。

`GameObject.hlsl` 直接读取 `gAlbedo/gFresnelR0/gRough`。批次按 Program、Material、Mesh 稳定排序，因此共享材质不会复制 CPU 对象，同时可以减少连续 Draw 的状态切换。

## 当前限制

顶点已有 TEXCOORD，但根签名没有 SRV/sampler，材质也没有纹理句柄，`texture` 尚未进入渲染路径。

关键源码：`Material`、`ResourceManager.cpp`、`DX12MaterialManager.cpp`、`shader/GameObject.hlsl`。
