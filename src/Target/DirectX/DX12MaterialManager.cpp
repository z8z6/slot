//
// Created by zhou_zhengming on 2026/5/22.
//

#include "Target/DirectX/DX12MaterialManager.h"
#include "d3d12.h"
#include "UI/Material/Material.h"
#include "UI/Material/MaterialRegistry.h"

using namespace z8;

DX12Material::DX12Material(Material *M)
  : Albedo(M->Albedo), FresnelR0(M->FresnelR0), Rough(M->Rough)
{}

z8::DX12MaterialManager::DX12MaterialManager(DX12Render *R) : DX12Common(R), Buffer(R){}

void DX12MaterialManager::Init() {
  auto* Metal = MaterialRegistry::Instance().Get("Metal");
  DX12Material M = {Metal};
  Buffer.Init(sizeof(M));
  Buffer.Update(&M);
}