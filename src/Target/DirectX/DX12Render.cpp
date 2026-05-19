//
// Created by zhou_zhengming on 2026/5/8.
//

#include "Target/DirectX/DX12Render.h"
#include "Core/Application.h"
#include "Core/Window.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Shader.h"
#include "UI/Object/GameObject.h"
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

void z8::DX12Render::Init()
{
  Msaa.Init();
  Cmd.Init();
  SwapChain.Init();
  RenderTarget.InitDescriptor();
  DepthStencil.InitDescriptor();
  ConstBuf.InitDescriptor();

  Resize();

  Cmd.Reset();
  ConstBuf.InitBuffer();
  RootSignature.Init();
  MeshManager.Init();
  PSO.Init();
  Cmd.CloseAndExecute();
  Cmd.Synchronize();
}

void z8::DX12Render::Update()
{
  GetCamera()->UpdateView();
  GetObjects()->Update(GetCamera()->GetView(), GetCamera()->GetProj());
  memcpy(&ConstBuf.ConstBufCPU[0], GetObjects()->ConstBuf(), GetObjects()->ConstBufSize());
}

void z8::DX12Render::Draw()
{
  Ok(Cmd.Allocator->Reset());
  // 这里需要绑定渲染流水线
  Cmd.ResetWithPso();

  // Rtv 资源类型转换
  auto RenderBarrier = CD3DX12_RESOURCE_BARRIER::Transition(GetCurRtvBuf(),
                                                            D3D12_RESOURCE_STATE_PRESENT,
                                                            D3D12_RESOURCE_STATE_RENDER_TARGET);
  Cmd.List->ResourceBarrier(1, &RenderBarrier);

  // Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
  Cmd.List->RSSetViewports(1, &ScreenView);
  Cmd.List->RSSetScissorRects(1, &ScissorRect);

  // 清空缓冲区
  CreateDpt();
  RenderTarget.ClearBuffer();
  DepthStencil.ClearBuffer();

  // 设置要写入的缓冲区
  RenderTarget.Bind();

  ID3D12DescriptorHeap* descriptorHeaps[] = {ConstBuf.DptHeap.Get()};
  Cmd.List->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

  RootSignature.Bind();
  MeshManager.Bind();

  Cmd.List->SetGraphicsRootDescriptorTable(0, ConstBuf.DptHeap->GetGPUDescriptorHandleForHeapStart());

  // 绘制图形
  Cmd.List->DrawIndexedInstanced(GetObjects()->Mesh->ICount(), 1, 0, 0, 0);

  // Rtv 资源类型转换
  auto PresentBarrier = CD3DX12_RESOURCE_BARRIER::Transition(GetCurRtvBuf(),
                                                             D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                             D3D12_RESOURCE_STATE_PRESENT);
  Cmd.List->ResourceBarrier(1, &PresentBarrier);

  Cmd.CloseAndExecute();

  // 呈现当前缓冲区
  SwapChain.Present();
  // 切换缓冲区
  CurRtvId = ++CurRtvId % RtvBufCount;

  // 等待命令执行完毕
  Cmd.Synchronize();
}

void DX12Render::Resize()
{
  Cmd.Synchronize();
  Cmd.Reset();

  for (auto& i : RtvBuf)
    i.Reset();
  DepthStencil.ResetBuffer();

  // 调整 SwapChain 大小
  SwapChain.Resize();

  CreateRtv();
  DepthStencil.InitBuffer();
  Cmd.CloseAndExecute();
  Cmd.Synchronize();

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
