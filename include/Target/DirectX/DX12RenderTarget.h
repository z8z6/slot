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
  /** 多采样颜色缓冲；文字互操作仍直接包装单采样交换链 Buffer。 */
  ComPtr<ID3D12Resource> MsaaBuffer;
  ComPtr<ID3D12DescriptorHeap> DptHeap;
  unsigned DptSize = 0;
  D3D12_CPU_DESCRIPTOR_HANDLE Dpt;
  D3D12_CPU_DESCRIPTOR_HANDLE MsaaDpt;
  DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  int CurRtvId = 0;

  DX12RenderTarget(DX12Render* R) : DX12Common(R){}

  void InitDescriptor();
  void InitBuffer();
  void ClearBuffer() const;
  void ResetBuffer();
  /** 把多采样颜色解析到当前交换链缓冲，并交给 D3D11On12 叠加文字。 */
  void Resolve() const;
  void Transition(bool toPresent = true) const;
  void Swap();
  void Bind(bool needDepth = true) const;
  ID3D12Resource* GetBuffer() const;
};

}

