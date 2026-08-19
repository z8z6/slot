# 着色器与常量 ABI

Shader 描述文件、注册名、入口与 Target；ShaderProgram 将 VS、PS、深度和混合状态组合成一个绘制管线资源。内建阶段和 Program 由 `BuiltinShader` 中的具体 C++ 类固化资源 ID、编译入口与固定管线状态；`ResourceManager::RegisterBuiltinResources` 只负责所有权和依赖顺序。注册时先将 VS/PS 放入对应资源池，再以稳定的索引引用构造 Program；`DX12Render::Init` 随后统一编译全部 Shader。

编译器按 Target 自动选择：Shader Model 6.x 使用 DXC，旧版 Target 使用 FXC。DXC 路径支持 include、HLSL 2021、Debug 的 `-Zi/-Od` 和 Release 的 `-O3`。每个 DX12Render 拥有自己的 `DX12ShaderLibrary`，CPU 描述仍由 Application 的 ResourceManager 统一拥有。

## 根签名

| 参数 | 寄存器 | 形式 | 内容 |
| --- | --- | --- | --- |
| 0 | b0 | 根 CBV | World、WorldInvTranspose 及对象外观参数 |
| 1 | b1 | 根 CBV | Material |
| 2 | b2 | 根 CBV | 全局常量、最多 8 盏方向光 |
| 3 | t0 | SRV 描述符表 | Material 基础色纹理 |

全局常量依次包含转置后的 ViewProj、8 元素光源数组、有效光源数、环境光、相机位置、宿主屏幕尺寸、UI 坐标原点和时间。`DX12GlobalConst` 与 `asset/shader/Core/Const.hlsl` 必须保持字段顺序、类型宽度和 16 字节打包兼容；CBV 地址按 256 字节对齐。

3D Shader 对有效方向光逐一执行 Lambert 漫反射、Blinn-Phong 高光与 Schlick Fresnel并累加。材质存在基础色纹理时先通过 point-wrap sampler 采样 `t0`。CPU 每帧把 `World` 和 `WorldInvTranspose` 一起写入对象常量，使非均匀缩放后法线仍与曲面切线正交。

普通物体和 UI 对象共享 b0 的两个矩阵前缀；UI 外观字段从第 128 字节开始。`ObjectTransformConst`、`UIObjectConst` 与 `asset/shader/Core/Const.hlsl` 通过静态断言保持 ABI 一致。UI Shader 将世界矩阵结果视为像素坐标，再映射到 NDC 并翻转 Y；像素颜色来自每对象 `ObjectColor`，Panel 可区分背景和标题栏。

GoogleTest 会真实编译内建注册表中的全部 SM 6.0 Shader。

关键源码：`BuiltinShader.h`、`BuiltinShader.cpp`、`ResourceManager.cpp`、`BuiltinResource.h`、`DX12Shader.cpp`、`DX12RootSignature.cpp`、`DX12GlobalConst.cpp`、`shader`、`tests/Shader/DX12ShaderTest.cpp`。
