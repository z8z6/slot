//
// Created by zhou_zhengming on 2026/5/8.
//

#include "Target/DirectX/DX12Render.h"
#include "Core/Application.h"
#include "Core/Window.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Shader.h"
#include "Target/DirectX/DX12FloatingWindow.h"
#include "Object/Camera/Camera.h"
#include "UI/Layout/SceneNode.h"
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
  FloatingWindows.reset();
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
  UOBatch.Init(App->Layout.GetMainUO());
  TextRenderer.Init();
  FloatingWindows = std::make_unique<DX12FloatingWindowManager>(*this);
  App->Layout.ConsumeDirty();

  Cmd.CloseAndExecute();
  Cmd.Synchronize();
}

void DX12Render::Update()
{
  // 即时声明只在控件拓扑变化时重建 UI 常量缓冲；稳定帧复用原有 GPU 资源。
  bool topologyChanged = App->Layout.ConsumeDirty();
  topologyChanged = FloatingWindows->Reconcile(topologyChanged);
  if (topologyChanged)
    UOBatch.Init(App->Layout.GetMainUO());
  // 1. 更新相机坐标
  if (const auto *scene = App->Layout.GetSceneNode();
      scene && scene->Viewport().Width > 0.0f &&
      scene->Viewport().Height > 0.0f)
    GetCamera()->UpdateProj(scene->Viewport().Width /
                            scene->Viewport().Height);
  GetCamera()->Update(GetTimer());
  // 2. 更新全局常量
  GlobalConst.Update(this);
  // 3. 更新物体数据
  GOBatch.Update();
  UOBatch.Update();
  FloatingWindows->Update();
}

void z8::DX12Render::Draw()
{
  Ok(Cmd.Allocator->Reset());
  // 绑定渲染流水线
  Cmd.Reset();

  // 非 MSAA 路径直接渲染交换链；MSAA 路径保持交换链为 Present，直到 Resolve。
  if (!Msaa.EnableMsaa)
    RenderTarget.Transition(false);

  // This needs to be reset whenever the command list is reset.
  Cmd.List->RSSetViewports(1, &ScreenView);
  Cmd.List->RSSetScissorRects(1, &ScissorRect);

  RenderTarget.Swap();
  RenderTarget.ClearBuffer();

  // 3D 与 UI 共享同一个 4x 颜色缓冲。SceneNode 的 viewport/scissor 将 3D
  // 限制在中央内容区，随后恢复全屏状态叠加 UI，不再需要不兼容 MSAA 的复制。
  if (const auto *scene = App->Layout.GetSceneNode()) {
    const auto &viewport = scene->Viewport();
    const auto left = static_cast<LONG>((std::max)(0.0f, viewport.Left));
    const auto top = static_cast<LONG>((std::max)(0.0f, viewport.Top));
    const auto right = static_cast<LONG>((std::min)(
        static_cast<float>(GetWindow()->Width), viewport.Left + viewport.Width));
    const auto bottom = static_cast<LONG>((std::min)(
        static_cast<float>(GetWindow()->Height), viewport.Top + viewport.Height));
    if (right > left && bottom > top) {
      DepthStencil.ClearBuffer();
      const D3D12_VIEWPORT sceneView{
          static_cast<float>(left), static_cast<float>(top),
          static_cast<float>(right - left), static_cast<float>(bottom - top),
          0.0f, 1.0f};
      const D3D12_RECT sceneScissor{left, top, right, bottom};
      Cmd.List->RSSetViewports(1, &sceneView);
      Cmd.List->RSSetScissorRects(1, &sceneScissor);
      RenderTarget.Bind();
      RootSignature.Bind();
      GOBatch.Draw();
    }
  }

  // UI 总是在场景合成之后覆盖交换链，工具面板不会被 3D 深度遮挡。
  Cmd.List->RSSetViewports(1, &ScreenView);
  Cmd.List->RSSetScissorRects(1, &ScissorRect);

  RenderTarget.Bind(false);

  RootSignature.Bind();

  UOBatch.Draw();

  if (Msaa.EnableMsaa)
    RenderTarget.Resolve();

  // DirectWrite 通过 D3D11On12 接管同一后备缓冲并完成 PRESENT 转换。
  Cmd.CloseAndExecute();
  Cmd.Synchronize();
  TextRenderer.Draw(App->Layout);

  SwapChain.Present();
  FloatingWindows->Draw();
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
