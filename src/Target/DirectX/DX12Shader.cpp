//
// Created by zhou_zhengming on 2026/5/11.
//
#include "Target/DirectX/DX12Shader.h"
#include <d3dcompiler.h>

using namespace z8;

DX12Shader::DX12Shader(Shader* s) : S(s){}


void DX12Shader::Compile()
{
  unsigned compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
  compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  ComPtr<ID3DBlob> errors;
  HRESULT hr = D3DCompileFromFile(S->FileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
          S->Entry.c_str(), S->Target.c_str(), compileFlags, 0, &ByteCode, &errors);

  if(errors != nullptr)
    OutputDebugStringA(static_cast<char *>(errors->GetBufferPointer()));

  Ok(hr);
}

D3D12_SHADER_BYTECODE DX12Shader::GetByteCode() const
{
  return {static_cast<BYTE*>(ByteCode->GetBufferPointer()), ByteCode->GetBufferSize()};
}


void DX12ShaderRegistry::Register(Shader* s) {
  auto* B = new DX12Shader(s);
  B->Compile();
  s->Binary = B;
  Shaders[s->Name] = s;
}


Shader * DX12ShaderRegistry::Get(std::string name)  {
  if (!Shaders.count(name)) return nullptr;
  return Shaders[name];
}
