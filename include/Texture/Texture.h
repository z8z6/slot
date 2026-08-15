#pragma once

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
class Texture {
public:
  std::string AssetId;
  uint32_t Height = 0;
  std::vector<std::byte> Pixels;
  uint32_t Width = 0;

  /** 返回 ResourceManager 自动注册使用的规范 ID。 */
  std::string GetName() const { return AssetId; }
  /** 使用 WIC 将 PNG/JPEG 等 Windows 支持格式统一解码为 RGBA8。 */
  bool Load(const std::wstring& path, std::string* error = nullptr);
  /** GPU 上传前验证尺寸和像素跨度，防止越界读取。 */
  bool Validate() const;
};

} // namespace z8
