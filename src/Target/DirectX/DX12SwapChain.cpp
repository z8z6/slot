//
// Created by zhou_zhengming on 2026/5/17.
//

#include "Target/DirectX/DX12SwapChain.h"
#include <dxgi1_4.h>
#include "Core/Window.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"

using namespace z8;

DX12SwapChain::DX12SwapChain(DX12Render* R) : DX12Common(R)
{
}

void z8::DX12SwapChain::Init()
{
  SwapChain.Reset();
  DXGI_SWAP_CHAIN_DESC SD;
  SD.BufferDesc.Width = Render->GetWindow()->Width;
  SD.BufferDesc.Height = Render->GetWindow()->Height;
  SD.BufferDesc.RefreshRate.Numerator = 60;
  SD.BufferDesc.RefreshRate.Denominator = 1;
  SD.BufferDesc.Format = Format;
  SD.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  SD.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  SD.SampleDesc.Count = Render->Msaa.GetSampleCount();
  SD.SampleDesc.Quality = Render->Msaa.GetMsaaQuality();
  SD.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  SD.BufferCount = RtvBufCount;
  SD.OutputWindow = Render->GetWindow()->Wnd;
  SD.Windowed = true;
  SD.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  SD.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  // SwapChain 创建依赖 CmdQueue
  ComPtr<IDXGISwapChain> SwapChain0;
  Ok(Ctx->Factory->CreateSwapChain(Render->Cmd.CmdQueue.Get(), &SD, SwapChain0.GetAddressOf()));
  Ok(SwapChain0.As(&SwapChain));
}

void DX12SwapChain::Resize()
{
  Ok(SwapChain->ResizeBuffers(
    RtvBufCount, Render->GetWindow()->Width, Render->GetWindow()->Height,
    Format, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
}

void DX12SwapChain::Present()
{
  Ok(SwapChain->Present(0, 0));
}

DX12SwapChain::SwapChainTy* DX12SwapChain::operator->() const
{
  return SwapChain.Get();
}


