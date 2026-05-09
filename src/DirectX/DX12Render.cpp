//
// Created by zhou_zhengming on 2026/5/8.
//

#include "Core/Window.h"
#include "DirectX/DX12Context.h"
#include "DirectX/DX12Render.h"
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <DirectXColors.h>

using namespace DirectX;
using namespace z8;

z8::DX12Render::DX12Render(Window *w) : Wnd(w) {
  Ctx = &DX12Context::Instance();
}

void z8::DX12Render::Init() {

  Ok(Ctx->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));

  // 查询堆描述符的大小
  RtvDescriptorSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  DsvDescriptorSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
  CbvSrvUavDescriptorSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  // 查询 MSAA 的支持情况
  D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
  msQualityLevels.Format = FormatRtv;
  msQualityLevels.SampleCount = 4;
  msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
  msQualityLevels.NumQualityLevels = 0;
  Ok(Ctx->Device->CheckFeatureSupport(
          D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
          &msQualityLevels,
          sizeof(msQualityLevels)));

  MsaaQuality = msQualityLevels.NumQualityLevels;
  assert(MsaaQuality > 0 && "Unexpected MSAA quality level.");

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
  CreateSwapChain();

  // Rtv Heap
  D3D12_DESCRIPTOR_HEAP_DESC RtvDesc;
  RtvDesc.NumDescriptors = RtvBufCount;
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

  Resize();
}

void z8::DX12Render::Update() {

}

void z8::DX12Render::Draw() {

  Ok(CmdAllocator->Reset());
  Ok(CmdList->Reset(CmdAllocator.Get(), nullptr));

  // Rtv 资源类型转换
  auto RenderBarrier = CD3DX12_RESOURCE_BARRIER::Transition(GetCurRtvBuf(),
          D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
  CmdList->ResourceBarrier(1, &RenderBarrier);

  // Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
  CmdList->RSSetViewports(1, &ScreenViewport);
  CmdList->RSSetScissorRects(1, &ScissorRect);

  // 清空缓冲区
  UpdateHandle();
  CmdList->ClearRenderTargetView(RtvHandle, Colors::LightSteelBlue, 0, nullptr);
  CmdList->ClearDepthStencilView(DsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

  // 设置要写入的缓冲区
  CmdList->OMSetRenderTargets(1, &RtvHandle,
    true, &DsvHandle);

  // Rtv 资源类型转换
  auto PresentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(GetCurRtvBuf(),
          D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
  CmdList->ResourceBarrier(1, &PresentBarrier);

  Ok(CmdList->Close());

  // 执行渲染命令
  ID3D12CommandList* cmdsLists[] = { CmdList.Get() };
  CmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

  // 呈现当前缓冲区
  Ok(SwapChain->Present(0, 0));
  // 切换缓冲区
  CurRtvId = ++CurRtvId % RtvBufCount;

  // 等待命令执行完毕
  Sync();
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

void DX12Render::Resize() {
  Sync();

  Ok(CmdList->Reset(CmdAllocator.Get(), nullptr));

  // Release the previous resources we will be recreating.
  for (int i = 0; i < RtvBufCount; ++i)
    RtvBuf[i].Reset();
    DsvBuf.Reset();

  // Resize the swap chain.
  Ok(SwapChain->ResizeBuffers(
	      RtvBufCount, Wnd->Width, Wnd->Height,
	      FormatRtv, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

  CurRtvId = 0;

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(RtvHeap->GetCPUDescriptorHandleForHeapStart());
  for (UINT i = 0; i < RtvBufCount; i++)
  {
    Ok(SwapChain->GetBuffer(i, IID_PPV_ARGS(&RtvBuf[i])));
    Ctx->Device->CreateRenderTargetView(RtvBuf[i].Get(), nullptr, rtvHeapHandle);
    rtvHeapHandle.Offset(1, RtvDescriptorSize);
  }

  // Create the depth/stencil buffer and view.
  D3D12_RESOURCE_DESC DsvBufDesc;
  DsvBufDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  DsvBufDesc.Alignment = 0;
  DsvBufDesc.Width = Wnd->Width;
  DsvBufDesc.Height = Wnd->Height;
  DsvBufDesc.DepthOrArraySize = 1;
  DsvBufDesc.MipLevels = 1;
  DsvBufDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
  DsvBufDesc.SampleDesc.Count = EnableMsaa ? 4 : 1;
  DsvBufDesc.SampleDesc.Quality = EnableMsaa ? (MsaaQuality - 1) : 0;
  DsvBufDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  DsvBufDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

  D3D12_CLEAR_VALUE optClear;
  optClear.Format = FormatDsv;
  optClear.DepthStencil.Depth = 1.0f;
  optClear.DepthStencil.Stencil = 0;
  Ok(Ctx->Device->CreateCommittedResource(
      &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
      D3D12_HEAP_FLAG_NONE,
      &DsvBufDesc,
      D3D12_RESOURCE_STATE_COMMON,
      &optClear,
      IID_PPV_ARGS(DsvBuf.GetAddressOf())));

  // Create descriptor to mip level 0 of entire resource using the format of the resource.
  D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
  dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
  dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  dsvDesc.Format = FormatDsv;
  dsvDesc.Texture2D.MipSlice = 0;
  UpdateHandle();
  Ctx->Device->CreateDepthStencilView(DsvBuf.Get(), &dsvDesc, DsvHandle);

  // Transition the resource from its initial state to be used as a depth buffer.
  CmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(DsvBuf.Get(),
	      D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));


  Ok(CmdList->Close());
  ID3D12CommandList* cmdsLists[] = { CmdList.Get() };
  CmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

  Sync();

  ScreenViewport.TopLeftX = 0;
  ScreenViewport.TopLeftY = 0;
  ScreenViewport.Width    = static_cast<float>(Wnd->Width);
  ScreenViewport.Height   = static_cast<float>(Wnd->Height);
  ScreenViewport.MinDepth = 0.0f;
  ScreenViewport.MaxDepth = 1.0f;

  ScissorRect = {0, 0, Wnd->Width, Wnd->Height};
}

void z8::DX12Render::CreateSwapChain() {
  SwapChain.Reset();
  DXGI_SWAP_CHAIN_DESC SwapDesc;
  SwapDesc.BufferDesc.Width = Wnd->Width;
  SwapDesc.BufferDesc.Height = Wnd->Height;
  SwapDesc.BufferDesc.RefreshRate.Numerator = 60;
  SwapDesc.BufferDesc.RefreshRate.Denominator = 1;
  SwapDesc.BufferDesc.Format = FormatRtv;
  SwapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  SwapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  SwapDesc.SampleDesc.Count = EnableMsaa ? 4 : 1;
  SwapDesc.SampleDesc.Quality = EnableMsaa ? (MsaaQuality - 1) : 0;
  SwapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  SwapDesc.BufferCount = RtvBufCount;
  SwapDesc.OutputWindow = Wnd->Wnd;
  SwapDesc.Windowed = true;
  SwapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  SwapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  // SwapChain 创建依赖 CmdQueue
  Ok(Ctx->Factory->CreateSwapChain(CmdQueue.Get(), &SwapDesc, SwapChain.GetAddressOf()));
}

void z8::DX12Render::UpdateHandle() {
  DsvHandle = DsvHeap->GetCPUDescriptorHandleForHeapStart();
  RtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(
          RtvHeap->GetCPUDescriptorHandleForHeapStart(),
          CurRtvId,
          RtvDescriptorSize);
}

ID3D12Resource *z8::DX12Render::GetCurRtvBuf() const {
  return RtvBuf[CurRtvId].Get();
}
