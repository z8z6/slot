# 材质

`Material` 包含 Albedo、FresnelR0、Rough、ShaderProgram 和可选 BaseColorTexture，资源身份由基类 `Resource::Id` 描述。GrassBlockMaterial 是 GameObject 默认材质，引用 `builtin://texture/grass-block` 的原创像素风草皮/泥土贴图；MetalMaterial 仍用于无纹理高反射表面。

`DX12Material` 保持与 HLSL `cbMaterial` 对应的字段布局，并上传基础色纹理存在标记。MaterialManager 为每种材质建立 256 字节对齐常量槽；TextureManager 把 RGBA8 像素上传到 DEFAULT heap，并在根参数 3 / `t0` 绑定对应 SRV。静态 `s0` 使用 point + wrap，保持像素边缘并支持 UV 平铺。

`GameObject.hlsl` 将采样的基础色与 Albedo 相乘，再参与环境光、Lambert 漫反射和 Blinn-Phong 高光。`Rough` 是 `[0, 1]` 的感知粗糙度：0 表示窄而锐利的高光，1 表示宽而柔和的高光。

批次按 Program、Material、Mesh 稳定排序，因此共享材质不会复制 CPU 对象，同时可以减少连续 Draw 的状态切换。

关键源码：`Material`、`Texture`、`ResourceManager.cpp`、`DX12MaterialManager.cpp`、`DX12TextureManager.cpp`、`asset/shader/GameObject.hlsl`；纹理资源统一放在 `asset/texture`。
