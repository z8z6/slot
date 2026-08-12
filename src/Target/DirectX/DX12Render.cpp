//
// Created by zhou_zhengming on 2026/5/8.
//

#include "Target/DirectX/DX12Render.h"
#include "Core/Application.h"
#include "Core/Window.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Shader.h"
#include "Object/Camera/Camera.h"
#include "Util/Math.h"
#include "d3dcompiler.h"
#include "d3dx12.h"
#include <dxgi1_4.h>


using namespace DirectX;
using namespace z8;

DX12Render::DX12Render(Application* app)
    : App(app), Cmd(this), SwapChain(this), Msaa(this), RootSignature(this),
      GOBatch(this), UOBatch(this, true), TextRenderer(this),
      DepthStencil(this), RenderTarget(this),
      MeshManager(this), MaterialManager(this), ShaderLibrary(app->Resources) {
  Ctx = &DX12Device::Instance();
}

DX12Render::~DX12Render() { Shutdown(); }

void DX12Render::Shutdown() {
  if (IsShutdown)
    return;
  IsShutdown = true;

  // 先等待 DX12 和 D3D11On12 对队列的所有提交完成，再断开文字互操作资源。
  // 此时 Application::Resources、Scene、Layout 和 Window 都仍然存活。
  if (Cmd.Queue && Cmd.Fence)
    Cmd.Synchronize();
  TextRenderer.Shutdown();
}

void DX12Render::Init()
{
  Msaa.Init();
  Cmd.Init();
  SwapChain.Init();
  RenderTarget.InitDescriptor();
  DepthStencil.InitDescriptor();

  Resize();

  Cmd.Reset();

  // 所有 Shader 在设备初始化阶段统一编译
  ShaderLibrary.CompileAll();
  RootSignature.Init();
  MeshManager.Init();
  MaterialManager.Init();

  GOBatch.Init(App->ActiveScene.GetGameObjects());
  UOBatch.Init(App->Layout.GetUO());
  TextRenderer.Init();
  App->Layout.ConsumeDirty();

  Cmd.CloseAndExecute();
  Cmd.Synchronize();
}

void DX12Render::Update()
{
  // 即时声明只在控件拓扑变化时重建 UI 常量缓冲；稳定帧复用原有 GPU 资源。
  if (App->Layout.ConsumeDirty())
    UOBatch.Init(App->Layout.GetUO());
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

  // DirectWrite 通过 D3D11On12 接管同一后备缓冲并完成 PRESENT 转换。
  Cmd.CloseAndExecute();
  Cmd.Synchronize();
  TextRenderer.Draw(App->Layout);

  SwapChain.Present();
}

void DX12Render::Resize()
{
  Cmd.Synchronize();
  Cmd.Reset();

  TextRenderer.PrepareResize();
  RenderTarget.ResetBuffer();
  DepthStencil.ResetBuffer();
  SwapChain.Resize();
  RenderTarget.InitBuffer();
  DepthStencil.InitBuffer();
  TextRenderer.Resize();

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
  return App->ActiveScene.GetCamera();
}

Window* DX12Render::GetWindow() const {
  return &App->Window;
}

Timer *DX12Render::GetTimer() const {
  return &App->Timer;
}
Light *DX12Render::GetLight() const {
  return App->ActiveScene.GetLight();
}
