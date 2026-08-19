#pragma once

#include "DX12Common.h"
#include "Resource/ResourceRef.h"
#include "Texture/BaseTexture.h"
#include "d3d12.h"

#include <unordered_map>
#include <vector>

namespace z8 {

/** 把 CPU RGBA8 Texture 上传为默认堆资源，并提供每材质使用的 SRV。 */
class DX12TextureManager : public DX12Common {
  ComPtr<ID3D12DescriptorHeap> DescriptorHeap;
  unsigned DescriptorSize = 0;
  std::unordered_map<ResourceRef<BaseTexture>, uint32_t,
                     ResourceRefHash<BaseTexture>> Indices;
  std::vector<ComPtr<ID3D12Resource>> Resources;
  // 上传缓冲至少保留到初始化命令执行并同步，避免 CopyTextureRegion 读取失效内存。
  std::vector<ComPtr<ID3D12Resource>> Uploads;

public:
  explicit DX12TextureManager(DX12Render* render);

  /** 绑定唯一的 CBV/SRV/UAV Shader-visible 堆。 */
  void Bind() const;
  /** 返回指定纹理的 GPU SRV；未知资源引用返回零描述符句柄。 */
  D3D12_GPU_DESCRIPTOR_HANDLE
  GetGPUDescriptor(ResourceRef<BaseTexture> texture) const;
  /** 上传 ResourceManager 中的全部纹理并创建稳定 SRV 索引。 */
  void Init();
};

} // namespace z8
