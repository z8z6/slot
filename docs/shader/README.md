# 着色器与常量 ABI

Shader 描述文件、注册名、入口与 Target；ShaderProgram 将 VS、PS、深度和混合状态组合成一个绘制管线资源。普通 Shader 不再创建 C++ 派生类：`asset/shader/*.shader.json` 是注册来源，CMake 生成显式注册代码，`DX12Render::Init` 再统一编译 ResourceManager 中的全部 Shader。

编译器按 Target 自动选择：Shader Model 6.x 使用 DXC，旧版 Target 使用 FXC。DXC 路径支持 include、HLSL 2021、Debug 的 `-Zi/-Od` 和 Release 的 `-O3`。每个 DX12Render 拥有自己的 `DX12ShaderLibrary`，CPU 描述仍由 Application 的 ResourceManager 统一拥有。

## 根签名

| 参数 | 寄存器 | 形式 | 内容 |
| --- | --- | --- | --- |
| 0 | b0 | CBV 描述符表 | World、WorldInvTranspose 及对象外观参数 |
| 1 | b1 | 根 CBV | Material |
| 2 | b2 | CBV 描述符表 | 全局常量 |

全局常量依次包含转置后的 ViewProj、方向光、环境光、相机位置、宿主屏幕尺寸、UI 坐标原点、帧耗时和累计时间。主交换链的 UI 原点为零；Floating PanelGroup 的独立交换链以 Group 左上角为原点，把同一 Layout 全局坐标映射到本地 HWND，而不改写控件常量。`DX12GlobalConst` 与 `asset/shader/Core/Const.hlsl` 必须保持字段顺序、类型宽度和 16 字节打包兼容；CBV 区间按 256 字节对齐。

3D Shader 使用 Lambert 漫反射、Blinn-Phong 高光与 Schlick Fresnel。感知粗糙度通过平方平滑度映射到 `[4, 96]` 高光指数，兼顾可见宽度和光滑表面的集中反射。CPU 每帧把 `World` 和 `WorldInvTranspose` 一起写入对象常量；顶点 Shader 使用后者变换法线，使非均匀缩放后法线仍与曲面切线正交。方向光向量在计算 `N·L` 前归一化，因此 `Light::Direction` 的长度不会意外改变光强。

普通物体和 UI 对象共享 b0 的两个矩阵前缀；UI 外观字段从第 128 字节开始。`ObjectTransformConst`、`UIObjectConst` 与 `asset/shader/Core/Const.hlsl` 通过静态断言保持 ABI 一致。UI Shader 将世界矩阵结果视为像素坐标，再映射到 NDC 并翻转 Y；像素颜色来自每对象 `ObjectColor`，Panel 可区分背景和标题栏。

GoogleTest 会真实编译生成注册表中的全部 SM 6.0 Shader，并额外验证 FXC 的 SM 5.0 兼容路径。

关键源码：`scripts/generate_shader_registry.py`、`DX12Shader.cpp`、`DX12RootSignature.cpp`、`DX12GlobalConst.cpp`、`shader`、`tests/Shader/DX12ShaderTest.cpp`。
