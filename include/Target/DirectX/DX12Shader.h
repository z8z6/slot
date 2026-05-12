//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once
#include "DX12Common.h"
#include <d3dcommon.h>
#include <string>
#include <vector>

namespace z8 {
class DX12Shader {
public:
  std::wstring Filename;
  std::string Name;
  std::string Entry;
  std::string Target;
  ComPtr<ID3DBlob> ByteCode;

  DX12Shader(std::wstring filename,std::string name, std::string entry, std::string target);

  void Compile();
};

class DX12ShaderRegistry
{
public:
  std::vector<DX12Shader> Shaders;

  DX12ShaderRegistry();
  void Prepare();
  DX12Shader* GetShader(std::string name);

  static DX12ShaderRegistry& Instance()
  {
    static DX12ShaderRegistry instance;
    return instance;
  }
};
}

