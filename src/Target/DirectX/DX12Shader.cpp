//
// Created by zhou_zhengming on 2026/5/11.
//
#include "Target/DirectX/DX12Shader.h"
#include "Util/Error.h"
#include <d3dcompiler.h>
#include <dxcapi.h>
#include <iostream>

using namespace z8;

DX12Shader::DX12Shader(Shader* s) : S(s){}


void DX12Shader::CompileByFxc()
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

void DX12Shader::CompileByDxc() {
  ComPtr<IDxcUtils> utils;
  ComPtr<IDxcCompiler3> compiler;
  ComPtr<IDxcIncludeHandler> includeHandler;

  // 创建 DXC 组件
  Ok(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));
  Ok(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));
  Ok(utils->CreateDefaultIncludeHandler(&includeHandler));

  // 读取文件内容
  ComPtr<IDxcBlobEncoding> sourceBlob;
  Ok(utils->LoadFile(S->FileName.c_str(), nullptr, &sourceBlob));

  // 准备 DxcBuffer
  DxcBuffer sourceBuffer;
  sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
  sourceBuffer.Size = sourceBlob->GetBufferSize();
  sourceBuffer.Encoding = DXC_CP_ACP;  // 使用 ANSI 编码

  // 准备编译参数
  // dxc.exe -E VS -T vs_6_0 .\Default.hlsl -Fo vs.cso
  // dxc.exe -E PS -T ps_6_0 .\Default.hlsl -Fo ps.cso
  std::wstring entryW = std::wstring(S->Entry.begin(), S->Entry.end());
  std::wstring targetW = std::wstring(S->Target.begin(), S->Target.end());

  std::vector<LPCWSTR> arguments;
  arguments.push_back(L"-E");
  arguments.push_back(entryW.c_str());
  arguments.push_back(L"-T");
  arguments.push_back(targetW.c_str());

  std::wcout << (L"Arguments:\n");
  for (auto& arg : arguments) {
    std::wcout <<(arg);
    std::wcout <<(L" ");
  }
  std::wcout <<(L"\n");
  return;

  // 编译
  ComPtr<IDxcResult> compileResult;
  Ok(compiler->Compile(
      &sourceBuffer,
      arguments.data(),
      static_cast<uint32_t>(arguments.size()),
      includeHandler.Get(),
      IID_PPV_ARGS(&compileResult)
  ));

  // 检查编译是否成功
  HRESULT status;
  Ok(compileResult->GetStatus(&status));
  Ok(status);

  // 获取编译结果
  ComPtr<IDxcBlobUtf16> err;
  Ok(compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&err), nullptr));

  if (err && err->GetStringLength() > 0)
  {
    // 输出错误信息
    std::wstring errorMsg(err->GetStringPointer());
    OutputDebugStringW(errorMsg.c_str());

    // 也可以转换为 ANSI 输出
    int len = WideCharToMultiByte(CP_ACP, 0, errorMsg.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string ansiError(len, 0);
    WideCharToMultiByte(CP_ACP, 0, errorMsg.c_str(), -1, &ansiError[0], len, nullptr, nullptr);
    OutputDebugStringA(ansiError.c_str());
  }


  if (SUCCEEDED(status)){
      // 获取字节码
      compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&ByteCode), nullptr);
  }


}

D3D12_SHADER_BYTECODE DX12Shader::GetByteCode() const
{
  return {static_cast<BYTE*>(ByteCode->GetBufferPointer()), ByteCode->GetBufferSize()};
}


void DX12ShaderRegistry::Register(Shader* s) {
  auto* B = new DX12Shader(s);
  B->CompileByFxc();
  //B->CompileByDxc();
  s->Binary = B;
  Shaders[s->Name] = s;
}


Shader * DX12ShaderRegistry::Get(std::string name)  {
  if (!Shaders.count(name)) return nullptr;
  return Shaders[name];
}
