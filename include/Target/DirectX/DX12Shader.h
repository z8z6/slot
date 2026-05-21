//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once
#include "DX12Common.h"
#include "UI/Shader/Shader.h"

#include <d3d12.h>
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
  D3D12_SHADER_BYTECODE GetByteCode() const;
};

class DX12ShaderRegistry
{
  DX12ShaderRegistry() = default;
public:
  std::vector<DX12Shader> Shaders;

  void Register(Shader s);
  DX12Shader* GetShader(std::string name);

  static DX12ShaderRegistry& Instance()
  {
    static DX12ShaderRegistry instance;
    return instance;
  }
};

template <typename ShaderTy>
class DX12ShaderRegister {
public:
  DX12ShaderRegister() {
    DX12ShaderRegistry::Instance().Register(ShaderTy());
  }
};

}

