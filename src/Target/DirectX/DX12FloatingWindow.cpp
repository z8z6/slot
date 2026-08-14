#include "Target/DirectX/DX12FloatingWindow.h"

#include "Core/Application.h"
#include "Core/Window.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"
#include "Target/DirectX/DX12RenderBatch.h"
#include "Target/DirectX/DX12TextRenderer.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/PanelGroupNode.h"
#include "UI/Style/Theme.h"
#include "Util/Color.h"
#include "d3dx12.h"

#include <WindowsX.h>
#include <algorithm>
#include <dwmapi.h>
#include <dxgi1_4.h>
#include <ranges>
#include <unordered_set>

using namespace z8;

namespace {

constexpr wchar_t FloatingWindowClass[] = L"SlotFloatingPanelWindow";
constexpr DWORD FloatingWindowStyle = WS_POPUP | WS_THICKFRAME;
constexpr DWORD FloatingWindowExtendedStyle = WS_EX_TOOLWINDOW;

COLORREF ToNativeColor(const DirectX::XMFLOAT4 &color) {
  const auto channel = [](float value) {
    return static_cast<BYTE>(std::clamp(value, 0.0f, 1.0f) * 255.0f + 0.5f);
  };
  return RGB(channel(color.x), channel(color.y), channel(color.z));
}

MouseButton ResolveMouseButton(UINT message, WPARAM wParam) {
  switch (message) {
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP:
    return MouseButton::Left;
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP:
    return MouseButton::Middle;
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP:
    return MouseButton::Right;
  case WM_XBUTTONDOWN:
  case WM_XBUTTONUP:
    return GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? MouseButton::X1
                                                  : MouseButton::X2;
  default:
    return MouseButton::None;
  }
}

void RegisterFloatingWindowClass() {
  static const bool registered = [] {
    WNDCLASSW windowClass{};
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = DefWindowProcW;
    windowClass.hInstance = Window::Instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.hbrBackground =
        static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    windowClass.lpszClassName = FloatingWindowClass;
    RegisterClassW(&windowClass);
    return true;
  }();
  (void)registered;
}

} // namespace

class DX12FloatingWindowManager::Host final {
private:
  static constexpr int BufferCount = 2;

  DX12Render *Render = nullptr;
  ui::PanelGroupNode *Group = nullptr;
  HWND Wnd = nullptr;
  ComPtr<IDXGISwapChain3> SwapChain;
  ComPtr<ID3D12Resource> Buffers[BufferCount];
  ComPtr<ID3D12Resource> MsaaBuffer;
  ComPtr<ID3D12DescriptorHeap> RtvHeap;
  D3D12_CPU_DESCRIPTOR_HANDLE BufferRtv[BufferCount]{};
  D3D12_CPU_DESCRIPTOR_HANDLE MsaaRtv{};
  std::unique_ptr<DX12RenderBatch> Batch;
  std::unique_ptr<DX12TextRenderer> TextRenderer;
  int Width = 1;
  int Height = 1;
  int BufferIndex = 0;
  bool ResizePending = false;
  bool SuppressWindowSync = false;
  bool ClosePending = false;
  bool InSizeMove = false;
  POINT LastMouse{};
  bool HasLastMouse = false;

  static LRESULT CALLBACK WindowProcedure(HWND window, UINT message,
                                          WPARAM wParam, LPARAM lParam) {
    auto *host =
        reinterpret_cast<Host *>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
      const auto *create = reinterpret_cast<CREATESTRUCTW *>(lParam);
      host = static_cast<Host *>(create->lpCreateParams);
      SetWindowLongPtrW(window, GWLP_USERDATA,
                        reinterpret_cast<LONG_PTR>(host));
    }
    return host ? host->HandleMessage(message, wParam, lParam)
                : DefWindowProcW(window, message, wParam, lParam);
  }

  void CreateBuffers() {
    BufferIndex = static_cast<int>(SwapChain->GetCurrentBackBufferIndex());
    const UINT step = Render->Ctx->Device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    auto handle = RtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (int index = 0; index < BufferCount; ++index) {
      Ok(SwapChain->GetBuffer(index, IID_PPV_ARGS(&Buffers[index])));
      BufferRtv[index] = handle;
      Render->Ctx->Device->CreateRenderTargetView(Buffers[index].Get(), nullptr,
                                                  handle);
      handle.ptr += step;
    }
    MsaaRtv = handle;

    D3D12_RESOURCE_DESC description{};
    description.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    description.Width = static_cast<UINT64>(Width);
    description.Height = static_cast<UINT>(Height);
    description.DepthOrArraySize = 1;
    description.MipLevels = 1;
    description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    description.SampleDesc.Count = Render->Msaa.GetSampleCount();
    description.SampleDesc.Quality = Render->Msaa.GetMsaaQuality();
    description.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    description.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = description.Format;
    clearValue.Color[0] = Color::EditorBackground.x;
    clearValue.Color[1] = Color::EditorBackground.y;
    clearValue.Color[2] = Color::EditorBackground.z;
    clearValue.Color[3] = Color::EditorBackground.w;
    const auto heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    Ok(Render->Ctx->Device->CreateCommittedResource(
        &heap, D3D12_HEAP_FLAG_NONE, &description,
        D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue,
        IID_PPV_ARGS(&MsaaBuffer)));
    Render->Ctx->Device->CreateRenderTargetView(MsaaBuffer.Get(), nullptr,
                                                MsaaRtv);

    ID3D12Resource *textBuffers[] = {Buffers[0].Get(), Buffers[1].Get()};
    TextRenderer = std::make_unique<DX12TextRenderer>(Render);
    TextRenderer->Init(textBuffers, DXGI_FORMAT_R8G8B8A8_UNORM);
  }

  void CreateDeviceResources() {
    DXGI_SWAP_CHAIN_DESC description{};
    description.BufferDesc.Width = static_cast<UINT>(Width);
    description.BufferDesc.Height = static_cast<UINT>(Height);
    description.BufferDesc.RefreshRate = {60, 1};
    description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    description.SampleDesc.Count = 1;
    description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    description.BufferCount = BufferCount;
    description.OutputWindow = Wnd;
    description.Windowed = TRUE;
    description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    description.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    ComPtr<IDXGISwapChain> baseSwapChain;
    Ok(Render->Ctx->Factory->CreateSwapChain(Render->Cmd.Queue.Get(),
                                             &description, &baseSwapChain));
    Ok(baseSwapChain.As(&SwapChain));

    D3D12_DESCRIPTOR_HEAP_DESC heapDescription{};
    heapDescription.NumDescriptors = BufferCount + 1;
    heapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    Ok(Render->Ctx->Device->CreateDescriptorHeap(&heapDescription,
                                                 IID_PPV_ARGS(&RtvHeap)));
    CreateBuffers();
  }

  void DestroyBuffers() {
    if (TextRenderer) {
      TextRenderer->Shutdown();
      TextRenderer.reset();
    }
    for (auto &buffer : Buffers)
      buffer.Reset();
    MsaaBuffer.Reset();
  }

  MouseMovArgs ResolveMouseArgs(UINT message, WPARAM wParam, LPARAM lParam) {
    POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    ClientToScreen(Wnd, &point);
    ScreenToClient(Render->App->Window.Wnd, &point);
    MouseMovArgs args;
    args.State = static_cast<unsigned>(GET_KEYSTATE_WPARAM(wParam));
    args.X = point.x;
    args.Y = point.y;
    args.Button = ResolveMouseButton(message, wParam);
    return args;
  }

  void RenderInteractiveResizeFrame() {
    if (!SwapChain || !Batch)
      return;
    // Floating HWND 进入自己的模态尺寸循环后，应用主循环同样不会运行；
    // 在消息内消费 ResizePending，确保文字按新 viewport 重排而非缩放旧帧。
    Resize();
    Update();
    Draw();
    // Floating HWND 的尺寸消息也会快于异步合成；同步当前 Present，避免
    // 新客户区右侧和底侧短暂显示窗口背景。
    DwmFlush();
  }

  LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CLOSE:
      ClosePending = true;
      return 0;
    case WM_ACTIVATE:
      if (LOWORD(wParam) != WA_INACTIVE && Group)
        Render->App->Layout.ActivateFloating(*Group);
      return 0;
    case WM_SIZE:
      if (wParam != SIZE_MINIMIZED) {
        Width = (std::max)(1, static_cast<int>(LOWORD(lParam)));
        Height = (std::max)(1, static_cast<int>(HIWORD(lParam)));
        ResizePending = true;
        if (!SuppressWindowSync)
          SynchronizeRectFromWindow();
        if (InSizeMove)
          RenderInteractiveResizeFrame();
      }
      return 0;
    case WM_ENTERSIZEMOVE:
      InSizeMove = true;
      return 0;
    case WM_EXITSIZEMOVE:
      InSizeMove = false;
      RenderInteractiveResizeFrame();
      return 0;
    case WM_MOVE:
      if (!SuppressWindowSync)
        SynchronizeRectFromWindow();
      return 0;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN: {
      SetCapture(Wnd);
      auto args = ResolveMouseArgs(message, wParam, lParam);
      Render->App->Layout.OnMouseDown(args);
      return 0;
    }
    case WM_MOUSEMOVE: {
      auto args = ResolveMouseArgs(message, wParam, lParam);
      args.DeltaX = HasLastMouse ? args.X - LastMouse.x : 0;
      args.DeltaY = HasLastMouse ? args.Y - LastMouse.y : 0;
      LastMouse = {args.X, args.Y};
      HasLastMouse = true;
      const bool dragging =
          (args.State & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 |
                         MK_XBUTTON2)) != 0;
      dragging ? Render->App->Layout.OnMouseDrag(args)
               : Render->App->Layout.OnMouseMove(args);
      return 0;
    }
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP: {
      auto args = ResolveMouseArgs(message, wParam, lParam);
      Render->App->Layout.OnMouseUp(args);
      if (GetCapture() == Wnd)
        ReleaseCapture();
      return 0;
    }
    case WM_MOUSEWHEEL: {
      POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
      ScreenToClient(Render->App->Window.Wnd, &point);
      Render->App->Layout.OnMouseWheel(
          {static_cast<unsigned>(GET_KEYSTATE_WPARAM(wParam)), point.x, point.y,
           GET_WHEEL_DELTA_WPARAM(wParam)});
      return 0;
    }
    case WM_KEYDOWN:
      Render->App->Layout.OnKeyDown(KeyArgs(wParam, lParam));
      return 0;
    case WM_KEYUP:
      Render->App->Layout.OnKeyUp(KeyArgs(wParam, lParam));
      return 0;
    case WM_ERASEBKGND:
      return 1;
    default:
      return DefWindowProcW(Wnd, message, wParam, lParam);
    }
  }

  void Resize() {
    if (!ResizePending || !SwapChain)
      return;
    ResizePending = false;
    Render->Cmd.Synchronize();
    DestroyBuffers();
    Ok(SwapChain->ResizeBuffers(
        BufferCount, static_cast<UINT>(Width), static_cast<UINT>(Height),
        DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
    CreateBuffers();
    RebuildBatch();
  }

  void SynchronizeRectFromWindow() {
    if (!Group || !Wnd)
      return;
    POINT origin{};
    ClientToScreen(Wnd, &origin);
    ScreenToClient(Render->App->Window.Wnd, &origin);
    const ui::DockRect rect{
        static_cast<float>(origin.x), static_cast<float>(origin.y),
        static_cast<float>(Width), static_cast<float>(Height)};
    if (Render->App->Layout.Dock.UpdateFloatingRect(*Group, rect))
      Render->App->Layout.Calculate(
          static_cast<float>(Render->App->Window.Width),
          static_cast<float>(Render->App->Window.Height));
  }

public:
  Host(DX12Render &render, ui::PanelGroupNode &group)
      : Render(&render), Group(&group) {
    RegisterFloatingWindowClass();
    const auto *state = Render->App->Layout.Dock.GetState(group);
    const auto rect = state ? state->FloatingRect : ui::DockRect{};
    Width = (std::max)(1, static_cast<int>(rect.Width));
    Height = (std::max)(1, static_cast<int>(rect.Height));
    POINT screenOrigin{static_cast<LONG>(rect.Left),
                       static_cast<LONG>(rect.Top)};
    ClientToScreen(Render->App->Window.Wnd, &screenOrigin);
    RECT frame{0, 0, Width, Height};
    AdjustWindowRectEx(&frame, FloatingWindowStyle, FALSE,
                       FloatingWindowExtendedStyle);
    Wnd = CreateWindowExW(FloatingWindowExtendedStyle, FloatingWindowClass,
                          L"Slot Panel", FloatingWindowStyle,
                          screenOrigin.x + frame.left,
                          screenOrigin.y + frame.top, frame.right - frame.left,
                          frame.bottom - frame.top, Render->App->Window.Wnd,
                          nullptr, Window::Instance, this);
    // WS_THICKFRAME 继续提供系统级缩放命中和阴影。Windows 仍会为该非客户区
    // 绘制顶边，因此将 Border/Caption 同步为 Panel 标题主题色，而不是依赖
    // 随系统浅色模式变化的默认白色。
    const COLORREF frameColor =
        ToNativeColor(ui::Theme::Default().Panel.TitleActiveColor);
    DwmSetWindowAttribute(Wnd, DWMWA_BORDER_COLOR, &frameColor,
                          sizeof(frameColor));
    DwmSetWindowAttribute(Wnd, DWMWA_CAPTION_COLOR, &frameColor,
                          sizeof(frameColor));
    SetWindowLongPtrW(Wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    SetWindowLongPtrW(Wnd, GWLP_WNDPROC,
                      reinterpret_cast<LONG_PTR>(WindowProcedure));
    CreateDeviceResources();
    RebuildBatch();
    ShowWindow(Wnd, SW_SHOWNOACTIVATE);
  }

  ~Host() {
    Render->Cmd.Synchronize();
    DestroyBuffers();
    Batch.reset();
    SwapChain.Reset();
    RtvHeap.Reset();
    if (Wnd && IsWindow(Wnd)) {
      SetWindowLongPtrW(Wnd, GWLP_USERDATA, 0);
      DestroyWindow(Wnd);
    }
  }

  ui::PanelGroupNode *GetGroup() const { return Group; }

  bool IsClosePending() const { return ClosePending; }

  void RebuildBatch() {
    Batch = std::make_unique<DX12RenderBatch>(Render, true);
    Batch->Init(Render->App->Layout.GetSubtreeUO(*Group));
  }

  void SynchronizeWindow() {
    const auto *state = Render->App->Layout.Dock.GetState(*Group);
    if (!state || state->Placement != ui::PanelPlacement::Floating)
      return;
    ui::DockRect rect = state->FloatingRect;
    if (Render->App->Layout.Dock.Drag.PayloadGroup == Group &&
        Render->App->Layout.Dock.Drag.State == ui::PanelDragState::Dragging)
      rect = Render->App->Layout.Dock.Drag.FloatingPreviewRect;
    POINT screenOrigin{static_cast<LONG>(rect.Left),
                       static_cast<LONG>(rect.Top)};
    ClientToScreen(Render->App->Window.Wnd, &screenOrigin);
    RECT frame{0, 0, static_cast<LONG>(rect.Width),
               static_cast<LONG>(rect.Height)};
    AdjustWindowRectEx(&frame, FloatingWindowStyle, FALSE,
                       FloatingWindowExtendedStyle);
    RECT currentFrame{};
    GetWindowRect(Wnd, &currentFrame);
    const int targetX = screenOrigin.x + frame.left;
    const int targetY = screenOrigin.y + frame.top;
    const int targetWidth = frame.right - frame.left;
    const int targetHeight = frame.bottom - frame.top;
    if (currentFrame.left == targetX && currentFrame.top == targetY &&
        currentFrame.right - currentFrame.left == targetWidth &&
        currentFrame.bottom - currentFrame.top == targetHeight)
      return;
    SuppressWindowSync = true;
    SetWindowPos(Wnd, nullptr, targetX, targetY, targetWidth, targetHeight,
                 SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
    SuppressWindowSync = false;
  }

  void Update() {
    Resize();
    Batch->Update();
    auto constants = Render->GlobalConst;
    constants.ScreenSize = {static_cast<float>(Width),
                            static_cast<float>(Height)};
    constants.UIOrigin = {Group->Left, Group->Top};
    constants.WriteToBatch(*Batch);
  }

  void Draw() {
    BufferIndex = static_cast<int>(SwapChain->GetCurrentBackBufferIndex());
    Ok(Render->Cmd.Allocator->Reset());
    Render->Cmd.Reset();
    const D3D12_VIEWPORT viewport{
        0.0f, 0.0f, static_cast<float>(Width), static_cast<float>(Height),
        0.0f, 1.0f};
    const D3D12_RECT scissor{0, 0, Width, Height};
    Render->Cmd.List->RSSetViewports(1, &viewport);
    Render->Cmd.List->RSSetScissorRects(1, &scissor);
    Render->Cmd.List->OMSetRenderTargets(1, &MsaaRtv, TRUE, nullptr);
    const float clear[] = {Color::EditorBackground.x, Color::EditorBackground.y,
                           Color::EditorBackground.z,
                           Color::EditorBackground.w};
    Render->Cmd.List->ClearRenderTargetView(MsaaRtv, clear, 0, nullptr);
    Render->RootSignature.Bind();
    Batch->Draw();
    D3D12_RESOURCE_BARRIER barriers[] = {
        CD3DX12_RESOURCE_BARRIER::Transition(
            MsaaBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_RESOLVE_SOURCE),
        CD3DX12_RESOURCE_BARRIER::Transition(
            Buffers[BufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RESOLVE_DEST)};
    Render->Cmd.List->ResourceBarrier(2, barriers);
    Render->Cmd.List->ResolveSubresource(Buffers[BufferIndex].Get(), 0,
                                         MsaaBuffer.Get(), 0,
                                         DXGI_FORMAT_R8G8B8A8_UNORM);
    barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
        MsaaBuffer.Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
        Buffers[BufferIndex].Get(), D3D12_RESOURCE_STATE_RESOLVE_DEST,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    Render->Cmd.List->ResourceBarrier(2, barriers);
    Render->Cmd.CloseAndExecute();
    Render->Cmd.Synchronize();
    TextRenderer->Draw(Render->App->Layout.GetSubtreeTexts(*Group), BufferIndex,
                       Group->Left, Group->Top);
    Ok(SwapChain->Present(0, 0));
  }
};

DX12FloatingWindowManager::DX12FloatingWindowManager(DX12Render &render)
    : Render(&render) {}

DX12FloatingWindowManager::~DX12FloatingWindowManager() = default;

void DX12FloatingWindowManager::Draw() {
  for (auto &host : Hosts)
    host->Draw();
}

bool DX12FloatingWindowManager::Reconcile(bool topologyChanged) {
  const auto floating = Render->App->Layout.Dock.GetFloatingPanels();
  std::unordered_set<ui::PanelGroupNode *> alive;
  for (auto *node : floating)
    if (auto *group = dynamic_cast<ui::PanelGroupNode *>(node))
      alive.insert(group);

  bool closesTree = false;
  for (const auto &host : Hosts)
    if (host->IsClosePending() && alive.contains(host->GetGroup())) {
      host->GetGroup()->CloseRequested = true;
      closesTree = true;
    }
  if (closesTree) {
    Render->App->Layout.RebuildIndex();
    return Reconcile(true);
  }

  std::erase_if(Hosts, [&alive](const auto &host) {
    return !alive.contains(host->GetGroup());
  });
  for (auto *group : alive) {
    const bool exists = std::ranges::any_of(
        Hosts, [group](const auto &host) { return host->GetGroup() == group; });
    if (!exists)
      Hosts.push_back(std::make_unique<Host>(*Render, *group));
  }
  if (topologyChanged)
    for (auto &host : Hosts)
      host->RebuildBatch();
  for (auto &host : Hosts)
    host->SynchronizeWindow();
  return topologyChanged;
}

void DX12FloatingWindowManager::Update() {
  for (auto &host : Hosts)
    host->Update();
}
