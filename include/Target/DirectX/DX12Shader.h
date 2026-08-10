//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once
#include "DX12Common.h"
#include "UI/Shader/Shader.h"

#include <d3d12.h>
#include <d3dcommon.h>
#include <string>
#include <memory>
#include <unordered_map>


namespace z8 {
class DX12Shader {
  Shader* Description;
public:
  ComPtr<ID3DBlob> ByteCode;
  DX12Shader(Shader* s);
  void Compile();
  void CompileByFxc();
  void CompileByDxc();
  D3D12_SHADER_BYTECODE GetByteCode() const;
};

class DX12ShaderRegistry
{
  DX12ShaderRegistry() = default;
public:
  std::unordered_map<std::string, std::unique_ptr<Shader>> Shaders;
  std::unordered_map<std::string, std::unique_ptr<DX12Shader>> Binaries;
  bool IsCompiled = false;

  void Register(std::unique_ptr<Shader> shader);
  void CompileAll();
  Shader* Get(const std::string& name);

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
    DX12ShaderRegistry::Instance().Register(std::make_unique<ShaderTy>());
  }
};

}

