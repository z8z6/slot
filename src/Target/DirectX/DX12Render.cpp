//
// Created by zhou_zhengming on 2026/5/8.
//

#include "Target/DirectX/DX12Render.h"
#include "Core/Application.h"
#include "Core/Window.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Shader.h"
#include "UI/Object/GameObject.h"
#include "Util/Color.h"
#include "Util/Math.h"
#include "d3dcompiler.h"
#include "d3dx12.h"
#include <dxgi1_4.h>

#include "UI/Mesh/Mesh.h"
#include "UI/Object/Camera.h"

using namespace DirectX;
using namespace z8;

z8::DX12Render::DX12Render(Application* app)
    : App(app), Cmd(this), SwapChain(this), Msaa(this), PSO(this), RootSignature(this),
      DepthStencil(this), RenderTarget(this), ConstBuf(this), MeshManager(this) {
  Ctx = &DX12Device::Instance();
}

DX12Render::~DX12Render()
{
  CmdSync();
}

void z8::DX12Render::Init()
{
  Ok(Ctx->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));

  Msaa.Init();
  CreateCmd();
  SwapChain.Init();
  RenderTarget.InitDescriptor();
  DepthStencil.InitDescriptor();
  ConstBuf.InitDescriptor();

  Resize();

  CmdBegin();
  ConstBuf.InitBuffer();
  RootSignature.Init();
  MeshManager.Init();
  PSO.Init();
  CmdEnd();
  CmdSync();
}

void z8::DX12Render::Update()
{
  GetCamera()->UpdateView();
  GetObjects()->Update(GetCamera()->GetView(), GetCamera()->GetProj());
  memcpy(&ConstBuf.ConstBufCPU[0], GetObjects()->ConstBuf(), GetObjects()->ConstBufSize());
}

void z8::DX12Render::Draw()
{
  Ok(CmdAllocator->Reset());
  // 这里需要绑定渲染流水线
  Ok(CmdList->Reset(CmdAllocator.Get(), PSO.Get()));

  // Rtv 资源类型转换
  auto RenderBarrier = CD3DX12_RESOURCE_BARRIER::Transition(GetCurRtvBuf(),
                                                            D3D12_RESOURCE_STATE_PRESENT,
                                                            D3D12_RESOURCE_STATE_RENDER_TARGET);
  CmdList->ResourceBarrier(1, &RenderBarrier);

  // Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
  CmdList->RSSetViewports(1, &ScreenView);
  CmdList->RSSetScissorRects(1, &ScissorRect);

  // 清空缓冲区
  CreateDpt();
  RenderTarget.ClearBuffer();
  DepthStencil.ClearBuffer();

  // 设置要写入的缓冲区
  RenderTarget.Bind();

  ID3D12DescriptorHeap* descriptorHeaps[] = {ConstBuf.DptHeap.Get()};
  CmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

  RootSignature.Bind();
  MeshManager.Bind();

  CmdList->SetGraphicsRootDescriptorTable(0, ConstBuf.DptHeap->GetGPUDescriptorHandleForHeapStart());

  // 绘制图形
  CmdList->DrawIndexedInstanced(GetObjects()->Mesh->ICount(), 1, 0, 0, 0);

  // Rtv 资源类型转换
  auto PresentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(GetCurRtvBuf(),
                                                             D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                             D3D12_RESOURCE_STATE_PRESENT);
  CmdList->ResourceBarrier(1, &PresentBarrier);

  CmdEnd();

  // 呈现当前缓冲区
  SwapChain.Present();
  // 切换缓冲区
  CurRtvId = ++CurRtvId % RtvBufCount;

  // 等待命令执行完毕
  CmdSync();
}

void z8::DX12Render::CmdSync()
{
  ++CurFence;
  Ok(CmdQueue->Signal(Fence.Get(), CurFence));
  if (Fence->GetCompletedValue() >= CurFence) return;

  HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);
  Ok(Fence->SetEventOnCompletion(CurFence, eventHandle));
  WaitForSingleObject(eventHandle, INFINITE);
  CloseHandle(eventHandle);
}

void DX12Render::CmdBegin()
{
  Ok(CmdList->Reset(CmdAllocator.Get(), nullptr));
}

void DX12Render::CmdEnd()
{
  Ok(CmdList->Close());

  // 执行渲染命令
  ID3D12CommandList* cmdsLists[] = {CmdList.Get()};
  CmdQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}


// 创建命令队列
// 初始化时调用一次
void DX12Render::CreateCmd()
{
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

void DX12Render::Resize()
{
  CmdSync();
  CmdBegin();

  for (auto& i : RtvBuf)
    i.Reset();
  DepthStencil.ResetBuffer();

  // 调整 SwapChain 大小
  SwapChain.Resize();

  CreateRtv();
  DepthStencil.InitBuffer();
  CmdEnd();
  CmdSync();

  ScreenView.TopLeftX = 0;
  ScreenView.TopLeftY = 0;
  ScreenView.Width = static_cast<float>(GetWindow()->Width);
  ScreenView.Height = static_cast<float>(GetWindow()->Height);
  ScreenView.MinDepth = 0.0f;
  ScreenView.MaxDepth = 1.0f;

  ScissorRect = {0, 0, GetWindow()->Width, GetWindow()->Height};

  GetCamera()->UpdateProj(GetWindow()->AspectRatio());
}

// 创建资源描述符
// 每次缓冲区交换时调用一次
void z8::DX12Render::CreateDpt()
{
  // 只需调用一次
  // DsvDpt = DsvDptHeap->GetCPUDescriptorHandleForHeapStart();
  // 计算描述符的偏移
  RenderTarget.Dpt = CD3DX12_CPU_DESCRIPTOR_HANDLE(
    RenderTarget.DptHeap->GetCPUDescriptorHandleForHeapStart(),
    CurRtvId,
    RenderTarget.DptSize);
}

// 创建 Rtv 缓冲区，并绑定描述符
// 每次 Resize 时调用一次
void DX12Render::CreateRtv()
{
  CurRtvId = 0;
  CD3DX12_CPU_DESCRIPTOR_HANDLE Dpt(RenderTarget.DptHeap->GetCPUDescriptorHandleForHeapStart());
  for (UINT i = 0; i < RtvBufCount; i++)
  {
    Ok(SwapChain->GetBuffer(i, IID_PPV_ARGS(&RtvBuf[i])));
    Ctx->Device->CreateRenderTargetView(RtvBuf[i].Get(), nullptr, Dpt);
    Dpt.Offset(1, RenderTarget.DptSize);
  }
}


ID3D12Resource* z8::DX12Render::GetCurRtvBuf() const
{
  return RtvBuf[CurRtvId].Get();
}

Camera* DX12Render::GetCamera() const
{
  return App->Camera;
}

GameObject* DX12Render::GetObjects() const
{
  return App->Objects[0];
}

Window* DX12Render::GetWindow() const {
  return &App->Window;
}
