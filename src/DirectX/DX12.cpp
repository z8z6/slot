//
// Created by zhou_zhengming on 2026/5/8.
//

#include "DirectX/DX12.h"
#include "Core/Window.h"

using namespace z8;

void z8::DX12::Init() {
  CreateDXGIFactory1(IID_PPV_ARGS(&Factory));
  D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Device));

  Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence));

  RtvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  DsvDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
  CbvSrvUavDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  // Command
  D3D12_COMMAND_QUEUE_DESC CmdQueueDesc = {};
  CmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  CmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  Device->CreateCommandQueue(&CmdQueueDesc, IID_PPV_ARGS(&CmdQueue));
  Device->CreateCommandAllocator(
          D3D12_COMMAND_LIST_TYPE_DIRECT,
          IID_PPV_ARGS(CmdAllocator.GetAddressOf()));
  Device->CreateCommandList(
          0,
          D3D12_COMMAND_LIST_TYPE_DIRECT,
          CmdAllocator.Get(),
          nullptr,
          IID_PPV_ARGS(CmdList.GetAddressOf()));
  CmdList->Close();

  // SwapChain
  SwapChain.Reset();
  DXGI_SWAP_CHAIN_DESC sd;
  sd.BufferDesc.Width = Window->Width;
  sd.BufferDesc.Height = Window->Height;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferDesc.Format = BackBufFormat;
  sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  sd.SampleDesc.Count = MsaaState ? 4 : 1;
  sd.SampleDesc.Quality = MsaaState ? (MsaaQuality - 1) : 0;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.BufferCount = SwapChainBufCount;
  sd.OutputWindow = Window->Wnd;
  sd.Windowed = true;
  sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  // SwapChain 创建依赖 CmdQueue
  Factory->CreateSwapChain(CmdQueue.Get(), &sd, SwapChain.GetAddressOf());

  // Heap
  D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
  rtvHeapDesc.NumDescriptors = SwapChainBufCount;
  rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  rtvHeapDesc.NodeMask = 0;
  Device->CreateDescriptorHeap(
      &rtvHeapDesc, IID_PPV_ARGS(RtvHeap.GetAddressOf()));

  D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
  dsvHeapDesc.NumDescriptors = 1;
  dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  dsvHeapDesc.NodeMask = 0;
  Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(DsvHeap.GetAddressOf()));
}