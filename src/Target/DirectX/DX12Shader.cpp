//
// Created by zhou_zhengming on 2026/5/11.
//
#include "Target/DirectX/DX12Shader.h"
#include <d3dcompiler.h>

using namespace z8;

DX12Shader::DX12Shader(std::wstring filename, std::string name, std::string entry, std::string target)
  : Filename(filename), Name(name), Entry(entry), Target(target)
{
}

void DX12Shader::Compile()
{
  unsigned compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
  compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  ComPtr<ID3DBlob> errors;
  HRESULT hr = D3DCompileFromFile(Filename.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
          Entry.c_str(), Target.c_str(), compileFlags, 0, &ByteCode, &errors);

  if(errors != nullptr)
    OutputDebugStringA(static_cast<char *>(errors->GetBufferPointer()));

  Ok(hr);
}

DX12ShaderRegistry::DX12ShaderRegistry()
{
  Prepare();
}

void DX12ShaderRegistry::Prepare()
{
  Shaders.emplace_back(L"shader\\Cube.hlsl", "DV", "VS", "vs_5_0");
  Shaders.emplace_back(L"shader\\Cube.hlsl", "DP", "PS", "ps_5_0");

  for (auto& Shader : Shaders)
    Shader.Compile();
}

DX12Shader* DX12ShaderRegistry::GetShader(std::string name)
{
  for (auto& Shader : Shaders)
  {
    if(Shader.Name == name)
      return &Shader;
  }
  return nullptr;
}
