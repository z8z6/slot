//
// Created by zhou_zhengming on 2026/5/18.
//

#pragma once

#include "DX12Common.h"
#include "d3d12.h"

namespace z8
{
class DX12RenderTarget: public DX12Common
{
  static constexpr int RtvBufCount = 2;
public:
  ComPtr<ID3D12Resource> Buffer[RtvBufCount];
  ComPtr<ID3D12DescriptorHeap> DptHeap;
  unsigned DptSize = 0;
  D3D12_CPU_DESCRIPTOR_HANDLE Dpt;
  DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  int CurRtvId = 0;

  DX12RenderTarget(DX12Render* R) : DX12Common(R){}

  void InitDescriptor();
  void InitBuffer();
  void ClearBuffer() const;
  void ResetBuffer();
  void Transition(bool toPresent = true) const;
  void Swap();
  void Bind(bool needDepth = true) const;
  ID3D12Resource* GetBuffer() const;
};

}

