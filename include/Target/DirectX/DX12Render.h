//
// Created by zhou_zhengming on 2026/5/8.
//
#pragma once

#include "DX12Command.h"
#include "DX12Common.h"
#include "DX12PipelineState.h"
#include "DX12RootSignature.h"
#include "DX12MeshManager.h"
#include "DX12Msaa.h"
#include "DX12RenderTarget.h"
#include "DX12ConstBuf.h"
#include "DX12DepthStencil.h"
#include "DX12SwapChain.h"
#include "Target/Render.h"
#include "d3d12.h"

namespace z8 {
class Camera;
class Window;
class GameObject;
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
  DX12ConstBuf ConstBuf;
  DX12MeshManager MeshManager;

  DX12Render(Application* app);

  void Init() override;
  void Update() override;
  void Draw() override;
  void Resize() override;

  Camera* GetCamera() const;
  GameObject* GetObjects() const;
  Window* GetWindow() const;
};

}




