#include "Target/DirectX/DX12FloatingWindow.h"

#include "Core/Application.h"
#include "Core/Win32WindowHost.h"
#include "Core/Window.h"
#include "Target/DirectX/DX12Render.h"
#include "Target/DirectX/DX12RenderBatch.h"
#include "Target/DirectX/DX12WindowSurface.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/PanelGroupNode.h"
#include "UI/Style/Theme.h"
#include "Util/Color.h"

#include <WindowsX.h>
#include <algorithm>
#include <dwmapi.h>
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

class DX12FloatingWindowManager::Host final : private Win32WindowHost {
private:
  DX12Render *Render = nullptr;
  ui::PanelGroupNode *Group = nullptr;
  HWND Wnd = nullptr;
  /** Floating HWND 独占呈现表面，但复用主渲染器的设备、队列与 PSO。 */
  DX12WindowSurface Surface;
  std::unique_ptr<DX12RenderBatch> Batch;
  int Width = 1;
  int Height = 1;
  bool ResizePending = false;
  bool SuppressWindowSync = false;
  bool ClosePending = false;
  bool InSizeMove = false;

  void RenderInteractiveResizeFrame() {
    if (!Surface.IsInitialized() || !Batch)
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

  LRESULT HandleWindowMessage(HWND window, UINT message, WPARAM wParam,
                              LPARAM lParam) override {
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
    case WM_ERASEBKGND:
      return 1;
    default: {
      LRESULT result = 0;
      if (DispatchInputMessage(window, message, wParam, lParam, result))
        return result;
      return DefWindowProcW(window, message, wParam, lParam);
    }
    }
  }

  void Resize() {
    if (!ResizePending || !Surface.IsInitialized())
      return;
    ResizePending = false;
    // Surface.Resize 会释放 D3D11On12 包装并重建交换链引用；调用前必须确保
    // 共享命令队列不再使用旧尺寸的 MSAA/后备缓冲。
    Render->Cmd.Synchronize();
    Surface.Resize(Width, Height);
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
      : Render(&render), Group(&group), Surface(&render) {
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
                          nullptr, Window::Instance, nullptr);
    // WS_THICKFRAME 继续提供系统级缩放命中和阴影。Windows 仍会为该非客户区
    // 绘制顶边，因此将 Border/Caption 同步为 Panel 标题主题色，而不是依赖
    // 随系统浅色模式变化的默认白色。
    const COLORREF frameColor =
        ToNativeColor(ui::Theme::Default().Panel.TitleActiveColor);
    DwmSetWindowAttribute(Wnd, DWMWA_BORDER_COLOR, &frameColor,
                          sizeof(frameColor));
    DwmSetWindowAttribute(Wnd, DWMWA_CAPTION_COLOR, &frameColor,
                          sizeof(frameColor));
    AttachWindow(Wnd, Render->App->Layout, Render->App->Window.Wnd);
    Surface.Init(Wnd, Width, Height, Render->Msaa.GetSampleCount(),
                 Render->Msaa.GetMsaaQuality(), Color::EditorBackground);
    RebuildBatch();
    ShowWindow(Wnd, SW_SHOWNOACTIVATE);
  }

  ~Host() {
    Render->Cmd.Synchronize();
    // Surface 内部按 DirectWrite → RTV → SwapChain 的逆依赖顺序释放资源。
    Surface.Shutdown();
    Batch.reset();
    if (Wnd && IsWindow(Wnd)) {
      DetachWindow();
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
    Ok(Render->Cmd.Allocator->Reset());
    Render->Cmd.Reset();
    Surface.PrepareFrame();
    Surface.ColorTarget.ClearBuffer();
    Surface.ColorTarget.Bind();
    Render->RootSignature.Bind();
    Batch->Draw();
    Surface.ColorTarget.Resolve();
    Render->Cmd.CloseAndExecute();
    Render->Cmd.Synchronize();
    Surface.DrawTextAndPresent(Render->App->Layout.GetSubtreeTexts(*Group),
                               Group->Left, Group->Top);
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
