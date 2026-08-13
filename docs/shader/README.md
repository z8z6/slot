# 着色器与常量 ABI

Shader 描述文件、注册名、入口与 Target；ShaderProgram 将 VS、PS、深度和混合状态组合成一个绘制管线资源。普通 Shader 不再创建 C++ 派生类：`asset/shader/*.shader.json` 是注册来源，CMake 生成显式注册代码，`DX12Render::Init` 再统一编译 ResourceManager 中的全部 Shader。

编译器按 Target 自动选择：Shader Model 6.x 使用 DXC，旧版 Target 使用 FXC。DXC 路径支持 include、HLSL 2021、Debug 的 `-Zi/-Od` 和 Release 的 `-O3`。每个 DX12Render 拥有自己的 `DX12ShaderLibrary`，CPU 描述仍由 Application 的 ResourceManager 统一拥有。

## 根签名

| 参数 | 寄存器 | 形式 | 内容 |
| --- | --- | --- | --- |
| 0 | b0 | CBV 描述符表 | World |
| 1 | b1 | 根 CBV | Material |
| 2 | b2 | CBV 描述符表 | 全局常量 |

全局常量依次包含转置后的 ViewProj、方向光、环境光、相机位置、屏幕尺寸、帧耗时和累计时间。`DX12GlobalConst` 与 `asset/shader/Core/Const.hlsl` 必须保持字段顺序、类型宽度和 16 字节打包兼容；CBV 区间按 256 字节对齐。

3D Shader 使用 Lambert 漫反射、Blinn-Phong 高光与 Schlick Fresnel。非均匀缩放下法线应使用世界矩阵逆转置，当前代码尚未处理。

UI Shader 将世界矩阵结果视为像素坐标，再映射到 NDC 并翻转 Y；像素颜色来自每对象 `ObjectColor`，Panel 可区分背景和标题栏。

GoogleTest 会真实编译生成注册表中的全部 SM 6.0 Shader，并额外验证 FXC 的 SM 5.0 兼容路径。

关键源码：`scripts/generate_shader_registry.py`、`DX12Shader.cpp`、`DX12RootSignature.cpp`、`DX12GlobalConst.cpp`、`shader`、`tests/Shader/DX12ShaderTest.cpp`。
