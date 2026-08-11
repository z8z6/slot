//
// Created by zhou_zhengming on 2026/5/21.
//

#pragma once
#include <string>

namespace z8 {
struct Shader {
  // Shader 只描述与后端无关的编译输入；DXIL 所有权位于 DX12ShaderLibrary。
  std::wstring FileName;
  std::string Name;
  std::string Target;
  std::string Entry;
};

}
