//
// Created by zhou_zhengming on 2026/5/8.
//
#pragma once

#include "DX12Command.h"
#include "DX12Common.h"
#include "DX12DepthStencil.h"
#include "DX12GlobalConst.h"
#include "DX12MaterialManager.h"
#include "DX12MeshManager.h"
#include "DX12Msaa.h"
#include "DX12PipelineState.h"
#include "DX12RenderBatch.h"
#include "DX12RenderTarget.h"
#include "DX12RootSignature.h"
#include "DX12Shader.h"
#include "DX12SwapChain.h"
#include "DX12TextRenderer.h"
#include "Target/Render.h"
#include "d3d12.h"

namespace z8 {
class Camera;
class Window;
class Timer;
class Light;
class Application;
class DX12FloatingWindowManager;

// 这个类是每个窗口独立的
class DX12Render : public Render {
public:
  Application* App;
  DX12Device* Ctx;

  D3D12_VIEWPORT ScreenView;
  D3D12_RECT ScissorRect;

  DX12Command Cmd;
  DX12SwapChain SwapChain;
  DX12Msaa Msaa;

  DX12DepthStencil DepthStencil;
  DX12RenderTarget RenderTarget;

  DX12MeshManager MeshManager;
  DX12MaterialManager MaterialManager;
  DX12GlobalConst GlobalConst;
  DX12RootSignature RootSignature;
  // ShaderLibrary 是每个渲染器的设备相关缓存，CPU 描述仍由 Application::Resources 拥有。
  DX12ShaderLibrary ShaderLibrary;

  DX12RenderBatch GOBatch;
  DX12RenderBatch UOBatch;
  DX12TextRenderer TextRenderer;
  /** Floating PanelGroup 的原生 HWND/交换链投影层。 */
  std::unique_ptr<DX12FloatingWindowManager> FloatingWindows;

  explicit DX12Render(Application* app);
  ~DX12Render() override;

  void Init() override;
  // 每帧绘制前调用，更新物体状态
  void Update() override;
  // 绘制物体
  void Draw() override;
  void Resize() override;
  void Shutdown() override;

  Camera* GetCamera() const;
  Window* GetWindow() const;
  Timer* GetTimer() const;
  Light* GetLight() const;

private:
  bool IsShutdown = false;
};

}




