#pragma once

#include "BaseShaderComponent.h"
#include "Resource/ResourceRef.h"

#include <string>
#include <string_view>

namespace z8 {

/**
 * @brief 一次图形绘制使用的完整 Shader 与固定管线状态描述。
 *
 * 场景对象只引用 Program，不分别绑定 VS/PS。深度和混合状态属于管线语义，放在
 * Program 中可避免后端通过对象 RTTI 猜测 UI 等特殊状态。
 */
struct BaseShader : Resource {
  using ShaderRef = ResourceRef<BaseShaderComponent>;
  ShaderRef VertexShader;
  ShaderRef PixelShader;
  bool EnableDepth = true;
  bool EnableBlend = false;

  BaseShader() {
    Type = ResourceTy::Shader;
    Id = builtin::shader::program::ShaderProgramPrefix;
  }
};

} // namespace z8
