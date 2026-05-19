//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once

#include "DX12Common.h"
#include "d3d12.h"

namespace z8
{
class DX12ConstBuf: public DX12Common
{
public:
  ComPtr<ID3D12Resource> Buffer;
  char* ConstBufCPU;
  ComPtr<ID3D12DescriptorHeap> DptHeap;
  unsigned DptSize = 0;
  D3D12_CPU_DESCRIPTOR_HANDLE Dpt;
  DXGI_FORMAT Format;

  DX12ConstBuf(DX12Render* R) : DX12Common(R){}
  ~DX12ConstBuf();

  void InitDescriptor();
  void InitBuffer();
};


}
