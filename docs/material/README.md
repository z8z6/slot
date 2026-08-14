# 材质

`Material` 包含 Albedo、FresnelR0、Rough 和 Name。MetalMaterial 是当前内建材质，GameObject 通过类型化软引用共享它。

`DX12Material` 保持与 HLSL `cbMaterial` 对应的字段布局。MaterialManager 上传 ResourceManager 中的全部材质，每个材质占用 256 字节对齐槽位；RenderItem 绘制前将对应 GPU 地址绑定到根参数 1 / HLSL `b1`。

`GameObject.hlsl` 直接读取 `gAlbedo/gFresnelR0/gRough`。`Rough` 是 `[0, 1]` 的感知粗糙度：0 表示窄而锐利的高光，1 表示宽而柔和的高光。Blinn-Phong Shader 将平滑度平方映射到 `[4, 96]` 的高光指数，避免旧的固定 `×256` 映射让默认高光在常见角度下数值性消失。内建 Metal 使用较高的 `FresnelR0`，使镜面光斑可以清楚辨认。

批次按 Program、Material、Mesh 稳定排序，因此共享材质不会复制 CPU 对象，同时可以减少连续 Draw 的状态切换。

## 当前限制

顶点已有 TEXCOORD，但根签名没有 SRV/sampler，材质也没有纹理句柄，`texture` 尚未进入渲染路径。

关键源码：`Material`、`ResourceManager.cpp`、`DX12MaterialManager.cpp`、`asset/shader/GameObject.hlsl`；文件材质资源统一放在 `asset/material`。
