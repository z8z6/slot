#pragma once

#include "BuiltinResource.h"
#include "Resource/ResourceHandle.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace z8 {

/**
 * @brief 与图形后端无关的 RGBA8 纹理资源。
 *
 * CPU 资源只保存解码后的像素和稳定资源名；DX12 上传资源、SRV 与状态转换由
 * DX12TextureManager 独占，避免资源层依赖设备生命周期。
 */
class BaseTexture : public Resource {
public:
  uint32_t Height = 0;
  uint32_t Width = 0;
  std::vector<std::byte> Pixels;

  BaseTexture() {
    Type = ResourceTy::Texture;
    Id = builtin::texture::TexturePrefix;
  }
  /** 使用 WIC 将 PNG/JPEG 等 Windows 支持格式统一解码为 RGBA8。 */
  bool Load(const std::wstring& path, std::string* error = nullptr);
  bool Validate() const;
};

} // namespace z8
