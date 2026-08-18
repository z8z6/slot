#pragma once

#include "BaseShaderProgram.h"

namespace z8 {

/** GameObject 像素阶段；固化内建资源身份和 PS 编译入口。 */
struct GameObjectPixelShader final : BaseShader {
  GameObjectPixelShader();
};

/** GameObject 管线组合；普通三维物体保留深度测试并关闭混合。 */
struct GameObjectShaderProgram final : BaseShaderProgram {
  GameObjectShaderProgram(ResourceHandle<BaseShader> vertexShader,
                          ResourceHandle<BaseShader> pixelShader);
};

/** GameObject 顶点阶段；固化内建资源身份和 VS 编译入口。 */
struct GameObjectVertexShader final : BaseShader {
  GameObjectVertexShader();
};

/** 缺失资源像素阶段；为后端提供稳定的错误可视化路径。 */
struct MissingPixelShader final : BaseShader {
  MissingPixelShader();
};

/** 缺失资源管线组合；保留深度语义以正确显示替代几何。 */
struct MissingShaderProgram final : BaseShaderProgram {
  MissingShaderProgram(ResourceHandle<BaseShader> vertexShader,
                       ResourceHandle<BaseShader> pixelShader);
};

/** 缺失资源顶点阶段；与对应像素阶段共享 HLSL 文件。 */
struct MissingVertexShader final : BaseShader {
  MissingVertexShader();
};

/** Time 效果像素阶段；将动态时间着色入口纳入内建类型系统。 */
struct TimePixelShader final : BaseShader {
  TimePixelShader();
};

/** Time 效果管线组合；作为三维效果保留深度并关闭混合。 */
struct TimeShaderProgram final : BaseShaderProgram {
  TimeShaderProgram(ResourceHandle<BaseShader> vertexShader,
                    ResourceHandle<BaseShader> pixelShader);
};

/** Time 效果顶点阶段；固化 Time HLSL 的 VS 编译描述。 */
struct TimeVertexShader final : BaseShader {
  TimeVertexShader();
};

/** UI 像素阶段；为原生控件表面提供透明度和边框着色入口。 */
struct UIObjectPixelShader final : BaseShader {
  UIObjectPixelShader();
};

/** UI 管线组合；关闭深度、开启混合，保证屏幕空间控件按层级合成。 */
struct UIObjectShaderProgram final : BaseShaderProgram {
  UIObjectShaderProgram(ResourceHandle<BaseShader> vertexShader,
                        ResourceHandle<BaseShader> pixelShader);
};

/** UI 顶点阶段；固化 UIObject HLSL 的 VS 编译描述。 */
struct UIObjectVertexShader final : BaseShader {
  UIObjectVertexShader();
};

} // namespace z8
