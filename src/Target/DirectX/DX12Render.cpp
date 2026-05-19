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
  // 绑定渲染流水线
  Cmd.ResetWithPso();

  RenderTarget.Transition(false);

  // This needs to be reset whenever the command list is reset.
  Cmd.List->RSSetViewports(1, &ScreenView);
  Cmd.List->RSSetScissorRects(1, &ScissorRect);

  RenderTarget.Swap();
  RenderTarget.ClearBuffer();
  DepthStencil.ClearBuffer();

  RenderTarget.Bind();

  ID3D12DescriptorHeap* descriptorHeaps[] = {ConstBuf.DptHeap.Get()};
  Cmd.List->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

  RootSignature.Bind();
  MeshManager.Bind();

  Cmd.List->SetGraphicsRootDescriptorTable(0, ConstBuf.DptHeap->GetGPUDescriptorHandleForHeapStart());

  // 绘制图形
  Cmd.List->DrawIndexedInstanced(GetObjects()->Mesh->ICount(), 1, 0, 0, 0);

  RenderTarget.Transition();

  Cmd.CloseAndExecute();
  SwapChain.Present();
  Cmd.Synchronize();
}

void DX12Render::Resize()
{
  Cmd.Synchronize();
  Cmd.Reset();

  RenderTarget.ResetBuffer();
  DepthStencil.ResetBuffer();
  SwapChain.Resize();
  RenderTarget.InitBuffer();
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
