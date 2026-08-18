//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once
#include "DX12Common.h"
#include "Resource/ResourceHandle.h"
#include "Shader/BaseShader.h"

#include <d3d12.h>
#include <d3dcommon.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace z8 {
class ResourceManager;
class DX12Shader {
  const BaseShader* Description;
public:
  ComPtr<ID3DBlob> ByteCode;
  explicit DX12Shader(const BaseShader* shader);
  void Compile();
  void CompileByFxc();
  void CompileByDxc();
  D3D12_SHADER_BYTECODE GetByteCode() const;
};

/**
 * @brief 单个 DX12Render 拥有的 Shader GPU 缓存。
 *
 * CPU Shader 由 ResourceManager 统一拥有；该类仅保存设备相关的编译产物，避免
 * Shader 资源反向依赖 DirectX 12。
 */
class DX12ShaderLibrary {
public:
  explicit DX12ShaderLibrary(ResourceManager& resources);

  bool IsCompiled = false;

  /** 编译当前 ResourceManager 中的所有 Shader；同一 Library 内重复调用直接复用。 */
  void CompileAll();
  /** 查询设备相关字节码，不拥有传入的 CPU Shader 句柄。 */
  DX12Shader* TryGet(ResourceHandle<BaseShader> handle) const;
  size_t Size() const { return Binaries.size(); }

private:
  ResourceManager* Resources;
  std::unordered_map<ResourceHandle<BaseShader>, std::unique_ptr<DX12Shader>,
                     ResourceHandleHash<BaseShader>> Binaries;
};

}

