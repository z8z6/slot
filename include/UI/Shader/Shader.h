//
// Created by zhou_zhengming on 2026/5/21.
//

#pragma once
#include <string>

namespace z8 {
class DX12Shader;
struct Shader {
  // Binary 由 DX12ShaderRegistry 统一拥有，在 CompileAll 后可用。
  DX12Shader* Binary = nullptr;
  std::wstring FileName;
  std::string Name;
  std::string Target;
  std::string Entry;
};

struct PixelShader: public Shader {
  PixelShader() {
    Entry = "PS";
    Target = "ps_6_0";
  }
};

struct VertexShader: public Shader {
  VertexShader() {
    Entry = "VS";
    Target = "vs_6_0";
  }
};
}
