#pragma once

#include "Target/DirectX/DX12Common.h"
#include "Target/DirectX/DX12RenderTarget.h"
#include "Target/DirectX/DX12SwapChain.h"
#include "Target/DirectX/DX12TextRenderer.h"

#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>

namespace z8::ui {
class TextNode;
}

namespace z8 {

/**
 * 一个 HWND 对应的完整颜色呈现表面。
 *
 * Surface 统一拥有交换链、后备缓冲 RTV、MSAA 颜色纹理和 DirectWrite 包装，
 * 并固化 ResizeBuffers 前后的释放顺序。Command queue、RootSignature 与 PSO 仍由
 * DX12Render 共享，避免每个 Floating window 重复创建全局设备资源。
 */
class DX12WindowSurface final : public DX12Common {
public:
  DX12SwapChain SwapChain;
  DX12RenderTarget ColorTarget;
  DX12TextRenderer TextRenderer;
  D3D12_VIEWPORT Viewport{};
  D3D12_RECT Scissor{};
  int Width = 1;
  int Height = 1;

private:
  DirectX::XMFLOAT4 ClearColor{};
  unsigned SampleCount = 1;
  unsigned SampleQuality = 0;
  bool Initialized = false;

public:
  explicit DX12WindowSurface(DX12Render *render);
  ~DX12WindowSurface() override;

  void ApplyViewport() const;
  void DrawTextAndPresent(const std::vector<ui::TextNode *> &texts,
                          float originX = 0.0f, float originY = 0.0f);
  void Init(HWND window, int width, int height, unsigned sampleCount,
            unsigned sampleQuality,
            const DirectX::XMFLOAT4 &clearColor);
  bool IsInitialized() const { return Initialized; }
  void PrepareFrame();
  /** 调用者需先同步共享命令队列，确保旧尺寸资源不再被 GPU 使用。 */
  void Resize(int width, int height);
  void Shutdown();

private:
  void RefreshViewport();
};

} // namespace z8
