//
// Created by zhou_zhengming on 2026/5/22.
//

#pragma once
#include "DX12Common.h"
#include "DX12UploadBuf.h"
#include <DirectXMath.h>


namespace z8 {
class Material;
struct DX12Material {
  DirectX::XMFLOAT4 Albedo;
  DirectX::XMFLOAT3 FresnelR0;
  float Rough = 0.25f;

  DX12Material(Material* M);
};
class DX12MaterialManager : public DX12Common{
public:
  DX12UploadBuf UploadBuf;
  ComPtr<ID3D12Resource> ConstBuf;

  DX12MaterialManager(DX12Render* R);
  void Init();
};
}

