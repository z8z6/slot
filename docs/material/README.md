# 材质

`Material` 包含 Albedo、FresnelR0、Rough 和 Name。MetalMaterial 是目前唯一注册材质，也是 GameObject 默认材质。

`DX12Material` 保持与 HLSL `cbMaterial` 对应的字段布局。MaterialManager 当前只上传一份名为 Metal 的材质，并以根 CBV 绑定到根参数 1 / HLSL `b1`。

## 当前限制

材质结构已经存在，但尚未真正驱动最终颜色：

- `GameObject::Material` 没有参与批次或材质索引选择。
- GPU 只上传一份 Metal。
- `GameObject.hlsl` 使用硬编码的 Albedo、FresnelR0 和 Rough，没有使用 b1 数据。

因此修改 C++ 材质目前不会改变最终 3D 颜色。完善时应先让 HLSL 使用 `gAlbedo/gFresnelR0/gRough`，再按材质拆批或建立材质表。

顶点已有 TEXCOORD，但根签名没有 SRV/sampler，材质也没有纹理句柄，`texture` 尚未进入渲染路径。

关键源码：`UI/Material`、`DX12MaterialManager.cpp`、`shader/GameObject.hlsl`。
