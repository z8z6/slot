//
// Created by zhou_zhengming on 2026/5/22.
//

#pragma once
#include "DX12Common.h"
#include "DX12DefaultBuffer.h"
#include "Material/Material.h"
#include "Resource/ResourceHandle.h"
#include <DirectXMath.h>
#include <unordered_map>


namespace z8 {
class Material;

struct DX12Material {
  DirectX::XMFLOAT4 Albedo;
  DirectX::XMFLOAT3 FresnelR0;
  float Rough = 0.25f;

  explicit DX12Material(const Material* material);
};

class DX12MaterialManager : public DX12Common{
public:
  DX12DefaultBuffer Buffer;

  DX12MaterialManager(DX12Render* R);
  void Init();
  /** 返回材质对齐槽位的根 CBV 地址；未知句柄返回 0，调用方不得绑定该地址。 */
  uint64_t GetGPUAddress(ResourceHandle<Material> material) const;

private:
  std::unordered_map<ResourceHandle<Material>, uint64_t,
                     ResourceHandleHash<Material>> Offsets;
};
}

