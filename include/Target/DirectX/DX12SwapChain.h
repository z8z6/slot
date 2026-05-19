//
// Created by zhou_zhengming on 2026/5/17.
//

#pragma once

#include <dxgiformat.h>
#include "DX12Common.h"

namespace z8
{
class DX12SwapChain : public DX12Common {
public:
  using SwapChainTy = IDXGISwapChain3;
  ComPtr<SwapChainTy> SwapChain;
  DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  static constexpr unsigned RtvBufCount = 2;

  DX12SwapChain(DX12Render* R);

  void Init();
  void Resize() const;
  void Present() const;
  SwapChainTy* operator->() const;
};
}



