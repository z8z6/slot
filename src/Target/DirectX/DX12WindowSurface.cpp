#include "Target/DirectX/DX12WindowSurface.h"

#include "Target/DirectX/DX12Render.h"

#include <algorithm>

using namespace z8;

DX12WindowSurface::DX12WindowSurface(DX12Render *render)
    : DX12Common(render), SwapChain(render), ColorTarget(render),
      TextRenderer(render) {}

DX12WindowSurface::~DX12WindowSurface() { Shutdown(); }

void DX12WindowSurface::ApplyViewport() const {
  Render->Cmd.List->RSSetViewports(1, &Viewport);
  Render->Cmd.List->RSSetScissorRects(1, &Scissor);
}

void DX12WindowSurface::DrawTextAndPresent(
    const std::vector<ui::TextNode *> &texts, float originX, float originY) {
  TextRenderer.Draw(texts, ColorTarget.CurrentBufferIndex, originX, originY);
  SwapChain.Present();
}

void DX12WindowSurface::Init(HWND window, int width, int height,
                             unsigned sampleCount, unsigned sampleQuality,
                             const DirectX::XMFLOAT4 &clearColor) {
  Shutdown();
  Width = (std::max)(1, width);
  Height = (std::max)(1, height);
  SampleCount = (std::max)(1U, sampleCount);
  SampleQuality = sampleQuality;
  ClearColor = clearColor;
  SwapChain.Init(window, Width, Height);
  ColorTarget.InitDescriptor();
  ColorTarget.InitBuffer(SwapChain, Width, Height, SampleCount, SampleQuality,
                         ClearColor);
  ID3D12Resource *buffers[] = {ColorTarget.Buffer[0].Get(),
                               ColorTarget.Buffer[1].Get()};
  TextRenderer.Init(buffers, SwapChain.Format);
  RefreshViewport();
  Initialized = true;
}

void DX12WindowSurface::PrepareFrame() {
  ColorTarget.SelectBuffer(SwapChain.GetCurrentBufferIndex());
  // MSAA 路径在 Resolve 时从 PRESENT 进入目标状态；单采样路径直接绘制
  // 后备缓冲，必须在首次 Clear/Draw 前显式转入 RENDER_TARGET。
  if (!ColorTarget.IsMultisampled())
    ColorTarget.Transition(false);
  ApplyViewport();
}

void DX12WindowSurface::RefreshViewport() {
  Viewport = {0.0f, 0.0f, static_cast<float>(Width),
              static_cast<float>(Height), 0.0f, 1.0f};
  Scissor = {0, 0, Width, Height};
}

void DX12WindowSurface::Resize(int width, int height) {
  if (!Initialized)
    return;
  Width = (std::max)(1, width);
  Height = (std::max)(1, height);
  // D3D11On12 包装必须先于交换链 COM 引用释放；顺序集中在这里后，主窗口
  // 与 Floating host 不会再维护两份容易漂移的 ResizeBuffers 协议。
  TextRenderer.PrepareResize();
  ColorTarget.ResetBuffer();
  SwapChain.Resize(Width, Height);
  ColorTarget.InitBuffer(SwapChain, Width, Height, SampleCount, SampleQuality,
                         ClearColor);
  ID3D12Resource *buffers[] = {ColorTarget.Buffer[0].Get(),
                               ColorTarget.Buffer[1].Get()};
  TextRenderer.Resize(buffers, SwapChain.Format);
  RefreshViewport();
}

void DX12WindowSurface::Shutdown() {
  if (!Initialized)
    return;
  // 文字目标持有后备缓冲的跨 API 引用，必须在颜色目标和交换链之前销毁。
  TextRenderer.Shutdown();
  ColorTarget.ResetBuffer();
  SwapChain.Reset();
  Initialized = false;
}
