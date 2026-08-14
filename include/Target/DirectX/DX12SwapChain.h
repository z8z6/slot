//
// Created by zhou_zhengming on 2026/5/17.
//

#pragma once

#include "DX12Common.h"
#include <dxgi1_4.h>
#include <windows.h>

namespace z8 {
/** 单个 HWND 的 Flip-model 交换链；尺寸和窗口由调用方显式提供。 */
class DX12SwapChain : public DX12Common {
public:
  using SwapChainTy = IDXGISwapChain3;

  ComPtr<SwapChainTy> SwapChain;
  DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  static constexpr int BufferCount = 2;

  explicit DX12SwapChain(DX12Render *render);

  int GetCurrentBufferIndex() const;
  void Init(HWND window, int width, int height);
  void Present() const;
  void Reset();
  void Resize(int width, int height) const;
  SwapChainTy *operator->() const;
};
} // namespace z8



