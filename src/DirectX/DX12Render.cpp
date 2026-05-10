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

DX12Render::~DX12Render() {
  Sync();
}

void z8::DX12Render::Init() {
  Ok(Ctx->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));

  CreateMsaa();
  CreateCmd();
  CreateSwapChain();
  CreateDptHeap();
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
  CmdList->RSSetViewports(1, &ScreenView);
  CmdList->RSSetScissorRects(1, &ScissorRect);

  // 清空缓冲区
  CreateDpt();
  CmdList->ClearRenderTargetView(RtvDpt, Colors::LightSteelBlue, 0, nullptr);
  CmdList->ClearDepthStencilView(DsvDpt, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

  // 设置要写入的缓冲区
  CmdList->OMSetRenderTargets(1, &RtvDpt,
    true, &DsvDpt);

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

// 查询 Msaa
// 初始化时调用一次
void DX12Render::CreateMsaa() {
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
}

// 创建命令队列
// 初始化时调用一次
void DX12Render::CreateCmd() {
  D3D12_COMMAND_QUEUE_DESC CD = {};
  CD.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  CD.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  Ok(Ctx->Device->CreateCommandQueue(&CD, IID_PPV_ARGS(&CmdQueue)));
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
}

void DX12Render::Resize() {
  Sync();

  Ok(CmdList->Reset(CmdAllocator.Get(), nullptr));

  for (int i = 0; i < RtvBufCount; ++i)
    RtvBuf[i].Reset();
  DsvBuf.Reset();

  // 调整 SwapChain 大小
  Ok(SwapChain->ResizeBuffers(
	      RtvBufCount, Wnd->Width, Wnd->Height,
	      FormatRtv, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

  CreateRtv();

  CreateDsv();

  Ok(CmdList->Close());
  ID3D12CommandList* cmdsLists[] = { CmdList.Get() };
  CmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

  Sync();

  ScreenView.TopLeftX = 0;
  ScreenView.TopLeftY = 0;
  ScreenView.Width    = static_cast<float>(Wnd->Width);
  ScreenView.Height   = static_cast<float>(Wnd->Height);
  ScreenView.MinDepth = 0.0f;
  ScreenView.MaxDepth = 1.0f;

  ScissorRect = {0, 0, Wnd->Width, Wnd->Height};
}

// 创建交换链
// 初始化时调用一次
void z8::DX12Render::CreateSwapChain() {
  SwapChain.Reset();
  DXGI_SWAP_CHAIN_DESC SD;
  SD.BufferDesc.Width = Wnd->Width;
  SD.BufferDesc.Height = Wnd->Height;
  SD.BufferDesc.RefreshRate.Numerator = 60;
  SD.BufferDesc.RefreshRate.Denominator = 1;
  SD.BufferDesc.Format = FormatRtv;
  SD.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  SD.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
  SD.SampleDesc.Count = EnableMsaa ? 4 : 1;
  SD.SampleDesc.Quality = EnableMsaa ? (MsaaQuality - 1) : 0;
  SD.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  SD.BufferCount = RtvBufCount;
  SD.OutputWindow = Wnd->Wnd;
  SD.Windowed = true;
  SD.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  SD.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

  // SwapChain 创建依赖 CmdQueue
  Ok(Ctx->Factory->CreateSwapChain(CmdQueue.Get(), &SD, SwapChain.GetAddressOf()));
}

// 创建资源描述符
// 每次缓冲区交换时调用一次
void z8::DX12Render::CreateDpt() {
  // 只需调用一次
  DsvDpt = DsvDptHeap->GetCPUDescriptorHandleForHeapStart();
  // 计算描述符的偏移
  RtvDpt = CD3DX12_CPU_DESCRIPTOR_HANDLE(
          RtvDptHeap->GetCPUDescriptorHandleForHeapStart(),
          CurRtvId,
          RtvDptSize);
}

// 创建 Dsv 缓冲区，并绑定描述符
// 每次 Resize 时调用一次
void DX12Render::CreateDsv() {

  // 创建 Dsv 缓冲区
  D3D12_RESOURCE_DESC BD;
  BD.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  BD.Alignment = 0;
  BD.Width = Wnd->Width;
  BD.Height = Wnd->Height;
  BD.DepthOrArraySize = 1;
  BD.MipLevels = 1;
  BD.Format = DXGI_FORMAT_R24G8_TYPELESS;
  BD.SampleDesc.Count = EnableMsaa ? 4 : 1;
  BD.SampleDesc.Quality = EnableMsaa ? (MsaaQuality - 1) : 0;
  BD.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  BD.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

  D3D12_CLEAR_VALUE Clv;
  Clv.Format = FormatDsv;
  Clv.DepthStencil.Depth = 1.0f;
  Clv.DepthStencil.Stencil = 0;

  auto HP = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
  Ok(Ctx->Device->CreateCommittedResource(
      &HP,
      D3D12_HEAP_FLAG_NONE,
      &BD,
      D3D12_RESOURCE_STATE_COMMON,
      &Clv,
      IID_PPV_ARGS(DsvBuf.GetAddressOf())));

  // 绑定 Dsv 描述符
  D3D12_DEPTH_STENCIL_VIEW_DESC DD;
  DD.Flags = D3D12_DSV_FLAG_NONE;
  DD.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
  DD.Format = FormatDsv;
  DD.Texture2D.MipSlice = 0;
  Ctx->Device->CreateDepthStencilView(DsvBuf.Get(), &DD, DsvDptHeap->GetCPUDescriptorHandleForHeapStart());

  // 状态转换
  auto WriteBarrier = CD3DX12_RESOURCE_BARRIER::Transition(DsvBuf.Get(),
              D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
  CmdList->ResourceBarrier(1, &WriteBarrier);
}

// 创建 Rtv 缓冲区，并绑定描述符
// 每次 Resize 时调用一次
void DX12Render::CreateRtv() {
  CurRtvId = 0;
  CD3DX12_CPU_DESCRIPTOR_HANDLE Dpt(RtvDptHeap->GetCPUDescriptorHandleForHeapStart());
  for (UINT i = 0; i < RtvBufCount; i++)
  {
    Ok(SwapChain->GetBuffer(i, IID_PPV_ARGS(&RtvBuf[i])));
    Ctx->Device->CreateRenderTargetView(RtvBuf[i].Get(), nullptr, Dpt);
    Dpt.Offset(1, RtvDptSize);
  }
}

// 创建资源描述符堆，存放描述符
// 初始化时调用一次
void DX12Render::CreateDptHeap() {
  // 查询堆描述符的大小
  RtvDptSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  DsvDptSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
  CbvSrvUavDptSize = Ctx->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  // Rtv 的描述符堆
  D3D12_DESCRIPTOR_HEAP_DESC RD;
  RD.NumDescriptors = RtvBufCount;  // 2个描述符
  RD.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  RD.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  RD.NodeMask = 0;
  Ok(Ctx->Device->CreateDescriptorHeap(&RD, IID_PPV_ARGS(RtvDptHeap.GetAddressOf())));

  // Dsv Heap
  D3D12_DESCRIPTOR_HEAP_DESC DD;
  DD.NumDescriptors = 1;
  DD.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
  DD.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  DD.NodeMask = 0;
  Ok(Ctx->Device->CreateDescriptorHeap(&DD, IID_PPV_ARGS(DsvDptHeap.GetAddressOf())));
}

ID3D12Resource *z8::DX12Render::GetCurRtvBuf() const {
  return RtvBuf[CurRtvId].Get();
}
