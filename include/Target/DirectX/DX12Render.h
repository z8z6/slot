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
  static const int RtvBufCount = 2;
  Application* App;
  DX12Device* Ctx;

  int CurRtvId = 0;
  ComPtr<ID3D12Resource> RtvBuf[RtvBufCount];
  DXGI_FORMAT FormatRtv = DXGI_FORMAT_R8G8B8A8_UNORM;

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

public:
  DX12Render(Application* app);
  ~DX12Render() override;

  void Init() override;
  void Update() override;
  void Draw() override;
  void Resize() override;

  void CreateDpt();
  void CreateRtv();
  ID3D12Resource* GetCurRtvBuf() const;

  Camera* GetCamera() const;
  GameObject* GetObjects() const;
  Window* GetWindow() const;
};

}




