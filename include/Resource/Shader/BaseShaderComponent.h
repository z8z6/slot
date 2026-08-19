//
// Created by zhou_zhengming on 2026/5/21.
//

#pragma once
#include "Resource/BuiltinResource.h"
#include "Resource/ResourceRef.h"

#include <string>
#include <string_view>

namespace z8 {
enum class ShaderTy {
  None,
  Vertex,
  Fragment,
  Compute,
  Pixel
};

/**
 * @brief Shader 派生类共享的后端无关编译描述。
 *
 * 内建派生类在 BuiltinShader 中固化 builtin ID、源文件、入口和
 * target；该基类只承载后端无关描述，DXIL 所有权仍位于
 * DX12ShaderLibrary。
 */
struct BaseShaderComponent : Resource {
  ShaderTy ShaderType;
  std::string Entry;
  std::wstring FileName;
  std::string Target;

  BaseShaderComponent() {
    ShaderType = ShaderTy::None;
    Type = ResourceTy::ShaderComponent;
    Id = builtin::shader::ShaderPrefix;
  }

};

struct VertexShaderComponent : BaseShaderComponent {
  VertexShaderComponent() {
    ShaderType = ShaderTy::Vertex;
    Entry = "VS";
    Target = "vs_6_0";
  }
};

struct PixelShaderComponent : BaseShaderComponent {
  PixelShaderComponent() {
    ShaderType = ShaderTy::Pixel;
    Entry = "PS";
    Target = "ps_6_0";
  }
};

} // namespace z8
