#include "Target/DirectX/DX12Shader.h"

#include "Util/Error.h"

#include <d3dcompiler.h>
#include <dxcapi.h>

#include <iostream>
#include <string>
#include <vector>

using namespace z8;

namespace {
void LogShaderMessage(const std::string& message) {
  std::clog << message << '\n';
  OutputDebugStringA((message + "\n").c_str());
}

std::string Narrow(const std::wstring& value) {
  if (value.empty()) return {};
  const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(),
                                       static_cast<int>(value.size()), nullptr, 0,
                                       nullptr, nullptr);
  std::string result(static_cast<size_t>(size), '\0');
  WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()),
                      result.data(), size, nullptr, nullptr);
  return result;
}
} // namespace

DX12Shader::DX12Shader(Shader* shader) : Description(shader) {}

void DX12Shader::Compile() {
  // Shader Model 6.x requires DXC; legacy targets remain available through FXC.
  if (Description->Target.find("_6_") != std::string::npos)
    CompileByDxc();
  else
    CompileByFxc();
}

void DX12Shader::CompileByFxc() {
  LogShaderMessage("[Shader][FXC] Compiling " + Description->Name + " (" +
                   Description->Target + ") from " + Narrow(Description->FileName));

  unsigned compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
  compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  ComPtr<ID3DBlob> errors;
  const HRESULT result = D3DCompileFromFile(
      Description->FileName.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
      Description->Entry.c_str(), Description->Target.c_str(), compileFlags, 0,
      ByteCode.ReleaseAndGetAddressOf(), errors.GetAddressOf());

  if (errors) {
    const std::string diagnostic(static_cast<const char*>(errors->GetBufferPointer()),
                                 errors->GetBufferSize());
    LogShaderMessage("[Shader][FXC] " + diagnostic);
  }
  Ok(result);
  LogShaderMessage("[Shader][FXC] Compiled " + Description->Name + " successfully.");
}

void DX12Shader::CompileByDxc() {
  LogShaderMessage("[Shader][DXC] Compiling " + Description->Name + " (" +
                   Description->Target + ") from " + Narrow(Description->FileName));

  ComPtr<IDxcUtils> utils;
  ComPtr<IDxcCompiler3> compiler;
  ComPtr<IDxcIncludeHandler> includeHandler;
  Ok(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));
  Ok(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));
  Ok(utils->CreateDefaultIncludeHandler(&includeHandler));

  ComPtr<IDxcBlobEncoding> source;
  Ok(utils->LoadFile(Description->FileName.c_str(), nullptr, &source));
  const DxcBuffer sourceBuffer{source->GetBufferPointer(), source->GetBufferSize(), DXC_CP_UTF8};

  const std::wstring entry(Description->Entry.begin(), Description->Entry.end());
  const std::wstring target(Description->Target.begin(), Description->Target.end());
  std::vector<LPCWSTR> arguments = {
      L"-E", entry.c_str(), L"-T", target.c_str(),
      L"-I", L"shader", L"-HV", L"2021"};
#if defined(DEBUG) || defined(_DEBUG)
  arguments.push_back(L"-Zi");
  arguments.push_back(L"-Od");
#else
  arguments.push_back(L"-O3");
#endif

  ComPtr<IDxcResult> compileResult;
  Ok(compiler->Compile(&sourceBuffer, arguments.data(),
                       static_cast<uint32_t>(arguments.size()), includeHandler.Get(),
                       IID_PPV_ARGS(&compileResult)));

  ComPtr<IDxcBlobUtf8> diagnostics;
  Ok(compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&diagnostics), nullptr));
  if (diagnostics && diagnostics->GetStringLength() > 0)
    LogShaderMessage("[Shader][DXC] " + std::string(diagnostics->GetStringPointer(),
                                                    diagnostics->GetStringLength()));

  HRESULT status = E_FAIL;
  Ok(compileResult->GetStatus(&status));
  Ok(status);

  ComPtr<IDxcBlob> object;
  Ok(compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&object), nullptr));
  Ok(object.As(&ByteCode));
  LogShaderMessage("[Shader][DXC] Compiled " + Description->Name + " successfully.");
}

D3D12_SHADER_BYTECODE DX12Shader::GetByteCode() const {
  return {ByteCode->GetBufferPointer(), ByteCode->GetBufferSize()};
}

void DX12ShaderRegistry::Register(std::unique_ptr<Shader> shader) {
  const std::string name = shader->Name;
  if (Shaders.contains(name))
    LogShaderMessage("[Shader][Warning] Replacing duplicate shader registration: " + name);
  Shaders[name] = std::move(shader);
  IsCompiled = false;
}

void DX12ShaderRegistry::CompileAll() {
  if (IsCompiled) {
    LogShaderMessage("[Shader] Compilation already completed; reusing bytecode.");
    return;
  }
  LogShaderMessage("[Shader] Starting unified shader compilation. Count: " +
                   std::to_string(Shaders.size()));
  Binaries.clear();
  for (auto& [name, shader] : Shaders) {
    auto binary = std::make_unique<DX12Shader>(shader.get());
    binary->Compile();
    shader->Binary = binary.get();
    Binaries[name] = std::move(binary);
  }
  IsCompiled = true;
  LogShaderMessage("[Shader] Unified shader compilation completed.");
}

Shader* DX12ShaderRegistry::Get(const std::string& name) {
  const auto it = Shaders.find(name);
  return it == Shaders.end() ? nullptr : it->second.get();
}
