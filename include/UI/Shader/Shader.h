//
// Created by zhou_zhengming on 2026/5/21.
//

#pragma once
#include <string>

namespace z8 {
class DX12Shader;
struct Shader {
  DX12Shader* Binary;
  std::wstring FileName;
  std::string Name;
  std::string Target;
  std::string Entry;
};

struct PixelShader: public Shader {
  PixelShader() {
    Entry = "PS";
    Target = "ps_5_0";
  }
};

struct VertexShader: public Shader {
  VertexShader() {
    Entry = "VS";
    Target = "vs_5_0";
  }
};
}
