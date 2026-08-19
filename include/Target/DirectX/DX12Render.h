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
#include "DX12RootSignature.h"
#include "DX12Shader.h"
#include "DX12TextureManager.h"
#include "DX12WindowSurface.h"
#include "Target/Render.h"
#include "d3d12.h"

#include <vector>

namespace z8 {
class BaseCamera;
class Window;
class Timer;
class BaseLight;
class Application;
class DX12FloatingWindowManager;

// 这个类是每个窗口独立的
class DX12Render : public Render {
public:
  Application* App;
  DX12Device* Ctx;

  DX12Command Cmd;
  DX12Msaa Msaa;
  /** 主 HWND 的呈现资源；Floating host 持有相同类型的独立实例。 */
  DX12WindowSurface WindowSurface;

  DX12DepthStencil DepthStencil;

  DX12MeshManager MeshManager;
  DX12MaterialManager MaterialManager;
  DX12TextureManager TextureManager;
  DX12GlobalConst GlobalConst;
  DX12RootSignature RootSignature;
  DX12ShaderLibrary ShaderLibrary;

  DX12RenderBatch GOBatch;
  DX12RenderBatch UOBatch;
  /** Floating PanelGroup 的原生 HWND/交换链投影层。 */
  std::unique_ptr<DX12FloatingWindowManager> FloatingWindows;

  explicit DX12Render(Application* app);
  ~DX12Render() override;

  void Init() override;
  void InvalidateSceneResources() override;
  // 每帧绘制前调用，更新物体状态
  void Update() override;
  // 绘制物体
  void Draw() override;
  void Resize() override;
  void Shutdown() override;

  BaseCamera* GetCamera() const;
  Window* GetWindow() const;
  Timer* GetTimer() const;
  /** 返回场景光源观察列表；所有权仍由 Scene 持有。 */
  const std::vector<BaseLight*>& GetLights() const;

private:
  bool IsShutdown = false;
  /** 延迟到帧边界重建场景批次，避免输入回调直接修改 GPU 资源。 */
  bool SceneResourcesDirty = false;
};

}




