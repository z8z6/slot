//
// Created by zhou_zhengming on 2026/5/18.
//

#pragma once

#include "DX12Common.h"
#include "d3d12.h"

namespace z8
{
class DX12DepthStencil : public DX12Common
{
public:
  ComPtr<ID3D12Resource> Buffer;
  ComPtr<ID3D12DescriptorHeap> DptHeap;
  unsigned DptSize = 0;
  D3D12_CPU_DESCRIPTOR_HANDLE Dpt;
  DXGI_FORMAT Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

  DX12DepthStencil(DX12Render* R) : DX12Common(R){}

  void InitDescriptor();
  void InitBuffer();
  void ClearBuffer();
  void ResetBuffer();
};
}

