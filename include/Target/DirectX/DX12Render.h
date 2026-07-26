//
// Created by zhou_zhengming on 2026/5/8.
//
#pragma once

#include "DX12Command.h"
#include "DX12Common.h"
#include "DX12ConstBuffer.h"
#include "DX12DepthStencil.h"
#include "DX12GlobalConst.h"
#include "DX12MaterialManager.h"
#include "DX12MeshManager.h"
#include "DX12Msaa.h"
#include "DX12PipelineState.h"
#include "DX12RenderObject.h"
#include "DX12RenderTarget.h"
#include "DX12RootSignature.h"
#include "DX12SwapChain.h"
#include "Target/Render.h"
#include "d3d12.h"

namespace z8 {
class Camera;
class Window;
class Timer;
class Light;
class Application;

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
  DX12PipelineState PSO;
  DX12RootSignature RootSignature;
  DX12DepthStencil DepthStencil;
  DX12RenderTarget RenderTarget;
  DX12ConstBuffer ConstBuffer;
  DX12MeshManager MeshManager;
  DX12MaterialManager MaterialManager;
  DX12GlobalConst GlobalConst;

  std::vector<DX12RenderObject> RenderObjects;

  DX12Render(Application* app);

  void Init() override;
  // 每帧绘制前调用，更新物体状态
  void Update() override;
  // 绘制物体
  void Draw() override;
  void Resize() override;

  void InitObject();
  Camera* GetCamera() const;
  Window* GetWindow() const;
  Timer* GetTimer() const;
  Light* GetLight() const;
};

}




