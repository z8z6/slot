#include "Target/DirectX/DX12Shader.h"

#include "Resource/ResourceManager.h"
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

DX12Shader::DX12Shader(const BaseShaderComponent* shader) : Description(shader) {}

void DX12Shader::Compile() {
  // Shader Model 6.x requires DXC; legacy targets remain available through FXC.
  if (Description->Target.find("_6_") != std::string::npos)
    CompileByDxc();
  else
    CompileByFxc();
}

void DX12Shader::CompileByFxc() {
  LogShaderMessage("[Shader][FXC] Compiling (" +
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
}

void DX12Shader::CompileByDxc() {
  LogShaderMessage("[Shader][DXC] Compiling (" +
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
      L"-I", L"asset/shader", L"-HV", L"2021"};
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
}

D3D12_SHADER_BYTECODE DX12Shader::GetByteCode() const {
  return {ByteCode->GetBufferPointer(), ByteCode->GetBufferSize()};
}

DX12ShaderLibrary::DX12ShaderLibrary(ResourceManager& resources)
    : Resources(&resources) {}

void DX12ShaderLibrary::CompileAll() {
  if (IsCompiled) {
    LogShaderMessage("[Shader] Compilation already completed; reusing bytecode.");
    return;
  }
  LogShaderMessage("[Shader] Starting unified shader compilation. Count: " +
                   std::to_string(Resources->Shaders.Size()));
  Binaries.clear();
  Resources->Shaders.Visit([this](ResourceRef<BaseShaderComponent> shaderRef,
                                       const BaseShaderComponent& shader) {
    auto binary = std::make_unique<DX12Shader>(&shader);
    binary->Compile();
    Binaries.emplace(shaderRef, std::move(binary));
  });
  IsCompiled = true;
  LogShaderMessage("[Shader] Unified shader compilation completed.");
}

DX12Shader* DX12ShaderLibrary::TryGet(ResourceRef<BaseShaderComponent> shader) const {
  const auto it = Binaries.find(shader);
  return it == Binaries.end() ? nullptr : it->second.get();
}
