//
// Created by zhou_zhengming on 2026/5/17.
//

#include "Target/DirectX/DX12SwapChain.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"
#include <algorithm>
#include <dxgi1_4.h>

using namespace z8;

DX12SwapChain::DX12SwapChain(DX12Render *render) : DX12Common(render) {}

int DX12SwapChain::GetCurrentBufferIndex() const {
  return static_cast<int>(SwapChain->GetCurrentBackBufferIndex());
}

void DX12SwapChain::Init(HWND window, int width, int height) {
  SwapChain.Reset();
  DXGI_SWAP_CHAIN_DESC description{};
  description.BufferDesc.Width = static_cast<UINT>((std::max)(1, width));
  description.BufferDesc.Height = static_cast<UINT>((std::max)(1, height));
  description.BufferDesc.RefreshRate = {60, 1};
  description.BufferDesc.Format = Format;
  description.BufferDesc.ScanlineOrdering =
      DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  description.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  // Flip 模型交换链只能是单采样；4x MSAA 在独立颜色缓冲完成后 Resolve。
  description.SampleDesc.Count = 1;
  description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  description.BufferCount = BufferCount;
  description.OutputWindow = window;
  description.Windowed = TRUE;
  description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  description.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  // SwapChain 创建依赖 CmdQueue。
  ComPtr<IDXGISwapChain> baseSwapChain;
  Ok(Ctx->Factory->CreateSwapChain(Render->Cmd.Queue.Get(), &description,
                                   baseSwapChain.GetAddressOf()));
  Ok(baseSwapChain.As(&SwapChain));
}

void DX12SwapChain::Present() const { Ok(SwapChain->Present(0, 0)); }

void DX12SwapChain::Reset() { SwapChain.Reset(); }

void DX12SwapChain::Resize(int width, int height) const {
  Ok(SwapChain->ResizeBuffers(
      BufferCount, static_cast<UINT>((std::max)(1, width)),
      static_cast<UINT>((std::max)(1, height)), Format,
      DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
}

DX12SwapChain::SwapChainTy *DX12SwapChain::operator->() const {
  return SwapChain.Get();
}


