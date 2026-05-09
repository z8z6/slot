//
// Created by zhou_zhengming on 2026/5/8.
//

#include "Core/Window.h"
#include "DirectX/DX12Context.h"
#include "DirectX/DX12Render.h"

z8::DX12Render::DX12Render(Window *w) : Wnd(w) {
  Ctx = &DX12Context::Instance();
}

void z8::DX12Render::Init() {

  Ok(Ctx->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));

  RtvDescriptorSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  DsvDescriptorSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
  CbvSrvUavDescriptorSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  // Command
  D3D12_COMMAND_QUEUE_DESC CmdQueueDesc = {};
  CmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  CmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  Ok(Ctx->Device->CreateCommandQueue(&CmdQueueDesc, IID_PPV_ARGS(&CmdQueue)));
  Ok(Ctx->Device->CreateCommandAllocator(
          D3D12_COMMAND_LIST_TYPE_DIRECT,
          IID_PPV_ARGS(CmdAllocator.GetAddressOf())));
  Ok(Ctx->Device->CreateCommandList(
          0,
          D3D12_COMMAND_LIST_TYPE_DIRECT,
          CmdAllocator.Get(),
          nullptr,
          IID_PPV_ARGS(CmdList.GetAddressOf())));
  Ok(CmdList->Close());

  // SwapChain
  SwapChain.Reset();
  DXGI_SWAP_CHAIN_DESC sd;
  sd.BufferDesc.Width = Wnd->Width;
  sd.BufferDesc.Height = Wnd->Height;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferDesc.Format = BackBufFormat;
  sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  sd.SampleDesc.Count = MsaaState ? 4 : 1;
  sd.SampleDesc.Quality = MsaaState ? (MsaaQuality - 1) : 0;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.BufferCount = SwapChainBufCount;
  sd.OutputWindow = Wnd->Wnd;
  sd.Windowed = true;
  sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  // SwapChain 创建依赖 CmdQueue
  Ok(Ctx->Factory->CreateSwapChain(CmdQueue.Get(), &sd, SwapChain.GetAddressOf()));

  // Rtv Heap
  D3D12_DESCRIPTOR_HEAP_DESC RtvDesc;
  RtvDesc.NumDescriptors = SwapChainBufCount;
  RtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  RtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  RtvDesc.NodeMask = 0;
  Ok(Ctx->Device->CreateDescriptorHeap(&RtvDesc, IID_PPV_ARGS(RtvHeap.GetAddressOf())));

  // Dsv Heap
  D3D12_DESCRIPTOR_HEAP_DESC DsvDesc;
  DsvDesc.NumDescriptors = 1;
  DsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  DsvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  DsvDesc.NodeMask = 0;
  Ok(Ctx->Device->CreateDescriptorHeap(&DsvDesc, IID_PPV_ARGS(DsvHeap.GetAddressOf())));
}

void z8::DX12Render::Sync() {
  ++CurFence;
  Ok(CmdQueue->Signal(Fence.Get(), CurFence));
  if(Fence->GetCompletedValue() >= CurFence) return;

  HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
  Ok(Fence->SetEventOnCompletion(CurFence, eventHandle));
  WaitForSingleObject(eventHandle, INFINITE);
  CloseHandle(eventHandle);
}

ID3D12Resource *z8::DX12Render::GetBackBuffer() const {
  return SwapChainBuf[CurBackBuffer].Get();
}