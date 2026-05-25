//
// Created by zhou_zhengming on 2026/5/11.
//
#include "Target/DirectX/DX12Shader.h"
#include "Util/Error.h"
#include <d3dcompiler.h>
#include <dxcapi.h>

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
  // 1. 创建 DXC 编译器实例
  ComPtr<IDxcUtils> dxcUtils;
  ComPtr<IDxcCompiler3> dxcCompiler;
  HRESULT hr;

  hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
  hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));

  // 2. 读取着色器文件
  ComPtr<IDxcBlobEncoding> shaderSource;
  hr = dxcUtils->LoadFile(S->FileName.c_str(), nullptr, &shaderSource);
  if (FAILED(hr))
  {
    OutputDebugStringA("Failed to load shader file!\n");
    Ok(hr);
    return;
  }

  // 3. 编译参数配置
  DxcBuffer sourceBuffer{};
  sourceBuffer.Ptr = shaderSource->GetBufferPointer();
  sourceBuffer.Size = shaderSource->GetBufferSize();
  sourceBuffer.Encoding = DXC_CP_UTF8;

  // 编译参数列表
  std::vector<LPCSTR> arguments;

  // 入口函数 + 着色器目标 (vs_6_0 / ps_6_0 等)
  arguments.push_back("-E");
  arguments.push_back(S->Entry.c_str());  // ANSI 转宽字符
  arguments.push_back("-T");
  arguments.push_back(S->Target.c_str()); // 支持 vs_6_0、ps_6_0、cs_6_0

  // Debug 模式：开启调试信息 + 关闭优化
#if defined(DEBUG) || defined(_DEBUG)
  arguments.push_back("-Zi");     // 调试信息
  arguments.push_back("-Od");     // 关闭优化
  arguments.push_back("-Qembed_debug"); // 嵌入调试信息到着色器
#endif

  // 4. 执行编译
  ComPtr<IDxcResult> compileResult;
  // hr = dxcCompiler->Compile(
  //     &sourceBuffer,
  //     arguments.data(),
  //     (UINT32)arguments.size(),
  //     nullptr,
  //     IID_PPV_ARGS(&compileResult)
  // );
  Ok(hr);

  // 5. 输出编译错误
  ComPtr<IDxcBlobUtf8> errors;
  compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
  if (errors != nullptr && errors->GetStringLength() > 0)
  {
    OutputDebugStringA((char*)errors->GetBufferPointer());
  }

  // 6. 获取编译成功的着色器字节码
  compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&ByteCode), nullptr);

  // // 7. 检查最终结果
  compileResult->GetStatus(&hr);
  Ok(hr);
}

D3D12_SHADER_BYTECODE DX12Shader::GetByteCode() const
{
  return {static_cast<BYTE*>(ByteCode->GetBufferPointer()), ByteCode->GetBufferSize()};
}


void DX12ShaderRegistry::Register(Shader* s) {
  auto* B = new DX12Shader(s);
  B->CompileByFxc();
  s->Binary = B;
  Shaders[s->Name] = s;
}


Shader * DX12ShaderRegistry::Get(std::string name)  {
  if (!Shaders.count(name)) return nullptr;
  return Shaders[name];
}
