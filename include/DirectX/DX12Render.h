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
  static const int RtvBufCount = 2;
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
  int CurRtvId = 0;
  ComPtr<IDXGISwapChain> SwapChain;
  // 这里是 GPU 的内存
  ComPtr<ID3D12Resource> RtvBuf[RtvBufCount];
  ComPtr<ID3D12Resource> DsvBuf;
  // 内存类型
  DXGI_FORMAT FormatRtv = DXGI_FORMAT_R8G8B8A8_UNORM;
  DXGI_FORMAT FormatDsv = DXGI_FORMAT_D24_UNORM_S8_UINT;
  // 存放 Resource 的若干描述符
  ComPtr<ID3D12DescriptorHeap> RtvHeap;
  ComPtr<ID3D12DescriptorHeap> DsvHeap;
  // 单个描述符的大小
  UINT RtvDescriptorSize = 0;
  UINT DsvDescriptorSize = 0;
  UINT CbvSrvUavDescriptorSize = 0;
  // 当前描述符的指针
  D3D12_CPU_DESCRIPTOR_HANDLE RtvHandle;
  D3D12_CPU_DESCRIPTOR_HANDLE DsvHandle;

  D3D_DRIVER_TYPE DriverType = D3D_DRIVER_TYPE_HARDWARE;

  D3D12_VIEWPORT ScreenViewport;
  D3D12_RECT ScissorRect;

  // MSAA
  bool EnableMsaa = false;
  UINT MsaaQuality = 0;

public:
  DX12Render(Window* w);
  void Init() override;
  void Update() override;
  void Draw() override;
  void Resize() override;

private:
  void Sync();
  void CreateSwapChain();
  void UpdateHandle();
  ID3D12Resource* GetCurRtvBuf() const;
};

}




