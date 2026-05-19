//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once
#include "DX12Common.h"

namespace z8
{
class DX12RootSignature : public DX12Common{
public:
  ComPtr<ID3D12RootSignature> RootSignature;
  DX12RootSignature(DX12Render* R);

  void Init();
  void Bind() const;
  ID3D12RootSignature* operator->() const;
  ID3D12RootSignature* Get() const;
};
}






