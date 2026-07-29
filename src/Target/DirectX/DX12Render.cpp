//
// Created by zhou_zhengming on 2026/5/8.
//

#include "Target/DirectX/DX12Render.h"
#include "Core/Application.h"
#include "Core/Window.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Shader.h"
#include "UI/Object/Camera/Camera.h"
#include "Util/Math.h"
#include "d3dcompiler.h"
#include "d3dx12.h"
#include <dxgi1_4.h>


using namespace DirectX;
using namespace z8;

DX12Render::DX12Render(Application* app)
    : App(app), Cmd(this), SwapChain(this), Msaa(this), RootSignature(this),
      GOBatch(this), UOBatch(this), DepthStencil(this), RenderTarget(this),
      MeshManager(this), MaterialManager(this) {
  Ctx = &DX12Device::Instance();
}

void z8::DX12Render::Init()
{
  Msaa.Init();
  Cmd.Init();
  SwapChain.Init();
  RenderTarget.InitDescriptor();
  DepthStencil.InitDescriptor();

  // GOBatch.Buffer.InitDescriptor();
  // UOBatch.Buffer.InitDescriptor();

  Resize();

  Cmd.Reset();

  RootSignature.Init();
  MeshManager.Init();
  MaterialManager.Init();

  GOBatch.Init(App->GOs);
  UOBatch.Init(App->Layout.UOs);

  Cmd.CloseAndExecute();
  Cmd.Synchronize();
}

void z8::DX12Render::Update()
{
  // 1. 更新相机坐标
  GetCamera()->Update(GetTimer());
  // 2. 更新全局常量
  GlobalConst.Update(this);
  // 3. 更新物体数据
  GOBatch.Update();
  UOBatch.Update();
}

void z8::DX12Render::Draw()
{
  Ok(Cmd.Allocator->Reset());
  // 绑定渲染流水线
  Cmd.Reset();

  RenderTarget.Transition(false);

  // This needs to be reset whenever the command list is reset.
  Cmd.List->RSSetViewports(1, &ScreenView);
  Cmd.List->RSSetScissorRects(1, &ScissorRect);

  RenderTarget.Swap();
  RenderTarget.ClearBuffer();
  DepthStencil.ClearBuffer();

  RenderTarget.Bind();

  RootSignature.Bind();

  GOBatch.Draw();
  UOBatch.Draw();

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


Camera * DX12Render::GetCamera() const
{
  return App->Camera;
}

Window* DX12Render::GetWindow() const {
  return &App->Window;
}

Timer *DX12Render::GetTimer() const {
  return &App->Timer;
}
Light *DX12Render::GetLight() const {
  return App->Light;
}
