//
// Created by zhou_zhengming on 2026/5/22.
//

#include "Target/DirectX/DX12MaterialManager.h"
#include "Core/Application.h"
#include "Material/BaseMaterial.h"
#include "Resource/ResourceManager.h"
#include "Target/DirectX/DX12Render.h"
#include "d3d12.h"

#include <cstring>
#include <vector>

using namespace z8;

DX12Material::DX12Material(const BaseMaterial* material)
  : Albedo(material->Albedo), FresnelR0(material->FresnelR0),
    Rough(material->Rough),
    HasBaseColorTexture(material->Texture.GetId().empty() ? 0U : 1U)
{}

z8::DX12MaterialManager::DX12MaterialManager(DX12Render *R) : DX12Common(R), Buffer(R){}

void DX12MaterialManager::Init() {
  constexpr uint64_t materialStride =
      (sizeof(DX12Material) + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1) &
      ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
  const size_t materialCount = Render->App->Resources.Materials.Size();
  std::vector<std::byte> data(materialCount * materialStride);
  Offsets.clear();

  uint64_t offset = 0;
  Render->App->Resources.Materials.Visit(
      [&](ResourceHandle<BaseMaterial> handle, const BaseMaterial& material) {
        const DX12Material gpuMaterial(&material);
        std::memcpy(data.data() + offset, &gpuMaterial, sizeof(gpuMaterial));
        Offsets.emplace(handle, offset);
        offset += materialStride;
      });

  // 根 CBV 地址必须按 256 字节对齐，因此每种材质占用独立对齐槽位。
  Buffer.Init(static_cast<unsigned>(data.size()));
  Buffer.Update(data.data());
}

uint64_t DX12MaterialManager::GetGPUAddress(
    ResourceHandle<BaseMaterial> material) const {
  const auto iterator = Offsets.find(material);
  if (iterator == Offsets.end() || !Buffer.DefaultBuffer) return 0;
  return Buffer.DefaultBuffer->GetGPUVirtualAddress() + iterator->second;
}
