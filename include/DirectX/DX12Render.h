//
// Created by zhou_zhengming on 2026/5/8.
//
#pragma once

#include "Core/IRender.h"
#include "DX12Common.h"
#include "d3d12.h"

namespace z8 {
class Window;
class DX12Context;

// 这个类是每个窗口独立的
class DX12Render : public IRender {
private:
  static const int SwapChainBufCount = 2;
  Window* Wnd;
  DX12Context* Ctx;

  // Sync
  ComPtr<ID3D12Fence> Fence;
  UINT64 CurFence = 0;

  // Command
  ComPtr<ID3D12CommandQueue> CmdQueue;
  ComPtr<ID3D12CommandAllocator> CmdAllocator;
  ComPtr<ID3D12GraphicsCommandList> CmdList;

  // Resource
  int CurBackBuffer = 0;
  ComPtr<IDXGISwapChain> SwapChain;
  ComPtr<ID3D12Resource> SwapChainBuf[SwapChainBufCount];
  ComPtr<ID3D12Resource> DepthBuf;

  ComPtr<ID3D12DescriptorHeap> RtvHeap;
  ComPtr<ID3D12DescriptorHeap> DsvHeap;
  UINT RtvDescriptorSize = 0;
  UINT DsvDescriptorSize = 0;
  UINT CbvSrvUavDescriptorSize = 0;

  D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;
  DXGI_FORMAT BackBufFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
  DXGI_FORMAT DepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

  D3D12_VIEWPORT ScreenViewport;
  D3D12_RECT ScissorRect;

  // MSAA
  bool MsaaState = false;
  UINT MsaaQuality = 0;

public:
  DX12Render(Window* w);
  void Init() override;
  void Sync();
  ID3D12Resource* GetBackBuffer() const;
};

}




