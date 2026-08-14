#include "Target/DirectX/DX12FloatingWindow.h"

#include "Core/Application.h"
#include "Core/Win32WindowHost.h"
#include "Core/Window.h"
#include "Target/DirectX/DX12DepthStencil.h"
#include "Target/DirectX/DX12Render.h"
#include "Target/DirectX/DX12RenderBatch.h"
#include "Target/DirectX/DX12WindowSurface.h"
#include "UI/Layout/BaseNode.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/PanelGroupNode.h"
#include "UI/Layout/SceneNode.h"
#include "UI/Style/Theme.h"
#include "Util/Color.h"

#include <WindowsX.h>
#include <algorithm>
#include <cmath>
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

/** 返回宿主子树中的场景节点；当前布局只渲染第一个可见场景视口。 */
ui::SceneNode *FindSceneNode(ui::BaseNode &root) {
  if (auto *scene = dynamic_cast<ui::SceneNode *>(&root))
    return scene;
  for (const auto &child : root.Children)
    if (auto *scene = FindSceneNode(*child))
      return scene;
  return nullptr;
}

} // namespace

class DX12FloatingWindowManager::Host final : private Win32WindowHost {
private:
  DX12Render *Render = nullptr;
  ui::BaseNode *Root = nullptr;
  /** 缓存创建时的节点类型，避免延迟回收窗口对悬空 Root 再次执行 RTTI。 */
  ui::PanelGroupNode *Group = nullptr;
  ui::SceneNode *Scene = nullptr;
  HWND Wnd = nullptr;
  /** Floating HWND 独占呈现表面，但复用主渲染器的设备、队列与 PSO。 */
  DX12WindowSurface Surface;
  /** 仅包含 SceneNode 的宿主创建深度目标，普通 Panel 不承担额外显存。 */
  DX12DepthStencil DepthTarget;
  std::unique_ptr<DX12RenderBatch> Batch;
  int Width = 1;
  int Height = 1;
  bool ResizePending = false;
  bool SuppressWindowSync = false;
  bool ClosePending = false;
  bool InSizeMove = false;

  /**
   * 判断非拥有 Root 是否仍由 Layout 持有。
   *
   * Panel 提交到其他 Group 时会在当前 WNDPROC 内销毁旧 Group，而 Host 必须
   * 延迟到下一帧 Reconcile 才能安全销毁 HWND。Nodes 已在提交末尾重建，
   * 所以纯地址查询可以封住这段窗口消息重入期，且不会解引用悬空指针。
   */
  bool IsRootAlive() const {
    return Root && Render && Render->App &&
           Render->App->Layout.ContainsNode(Root);
  }

  /**
   * 将 Floating HWND 的可见边缘统一交给 Win32 非客户区状态机。
   *
   * point 使用屏幕坐标；ScreenToClient 后允许出现负数和超出客户区的值，
   * 因而同一判断同时覆盖系统 frame 外侧与 UI 客户区内侧，消除两套 resize
   * 捕获在左/上边缘竞争导致的概率性失效。
   */
  LRESULT ResolveNonClientHit(LPARAM lParam) const {
    if (!Wnd || !IsRootAlive())
      return HTNOWHERE;
    POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    ScreenToClient(Wnd, &point);
    RECT client{};
    GetClientRect(Wnd, &client);
    const int border = (std::max)(
        1, static_cast<int>(std::ceil(ui::Theme::Default().Panel.ResizeBorder)));
    const bool left = point.x < border;
    const bool right = point.x >= client.right - border;
    const bool top = point.y < border;
    const bool bottom = point.y >= client.bottom - border;
    if (top && left)
      return HTTOPLEFT;
    if (top && right)
      return HTTOPRIGHT;
    if (bottom && left)
      return HTBOTTOMLEFT;
    if (bottom && right)
      return HTBOTTOMRIGHT;
    if (left)
      return HTLEFT;
    if (right)
      return HTRIGHT;
    if (top)
      return HTTOP;
    if (bottom)
      return HTBOTTOM;

    // Panel tab 仍走 Layout 的 Panel Dock 手势；只有 Tab 与关闭按钮之间的
    // 空白标题区作为原生 caption 移动整个 HWND，不产生 DockSession。
    if (Group && Group->DragHandleNode &&
        Group->DragHandleNode->Contains(Root->Left + point.x,
                                        Root->Top + point.y))
      return HTCAPTION;
    return HTCLIENT;
  }

  void RenderInteractiveResizeFrame() {
    if (!IsRootAlive() || !Surface.IsInitialized() || !Batch)
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
    case WM_NCHITTEST:
      return ResolveNonClientHit(lParam);
    case WM_CLOSE:
      // PanelGroup 有明确关闭语义；Scene 是场景唯一视口，Alt+F4 不应静默
      // 销毁业务节点，仍允许用户通过标题拖动重新停靠。
      ClosePending = IsRootAlive() && Group;
      return 0;
    case WM_ACTIVATE:
      if (LOWORD(wParam) != WA_INACTIVE && IsRootAlive())
        Render->App->Layout.ActivateFloating(*Root);
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
      // Dock 提交后的旧 HWND 可能在 Reconcile 前继续收到鼠标消息；此时不能
      // 再把输入路由给已重建的 Layout，也不能访问旧节点观察指针。
      if (!IsRootAlive())
        return DefWindowProcW(window, message, wParam, lParam);
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
    if (!IsRootAlive())
      return;
    // Surface.Resize 会释放 D3D11On12 包装并重建交换链引用；调用前必须确保
    // 共享命令队列不再使用旧尺寸的 MSAA/后备缓冲。
    Render->Cmd.Synchronize();
    Surface.Resize(Width, Height);
    if (Scene)
      DepthTarget.InitBuffer(Width, Height, Render->Msaa.GetSampleCount(),
                             Render->Msaa.GetMsaaQuality());
    RebuildBatch();
  }

  void SynchronizeRectFromWindow() {
    if (!IsRootAlive() || !Wnd)
      return;
    POINT origin{};
    ClientToScreen(Wnd, &origin);
    ScreenToClient(Render->App->Window.Wnd, &origin);
    const ui::DockRect rect{
        static_cast<float>(origin.x), static_cast<float>(origin.y),
        static_cast<float>(Width), static_cast<float>(Height)};
    if (Render->App->Layout.Dock.UpdateFloatingRect(*Root, rect))
      Render->App->Layout.Calculate(
          static_cast<float>(Render->App->Window.Width),
          static_cast<float>(Render->App->Window.Height));
  }

public:
  Host(DX12Render &render, ui::BaseNode &root)
      : Render(&render), Root(&root),
        Group(dynamic_cast<ui::PanelGroupNode *>(&root)),
        Scene(FindSceneNode(root)), Surface(&render), DepthTarget(&render) {
    RegisterFloatingWindowClass();
    const auto *state = Render->App->Layout.Dock.GetState(root);
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
                          Scene ? L"Slot Viewport" : L"Slot Panel",
                          FloatingWindowStyle,
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
    if (Scene) {
      DepthTarget.InitDescriptor();
      DepthTarget.InitBuffer(Width, Height, Render->Msaa.GetSampleCount(),
                             Render->Msaa.GetMsaaQuality());
    }
    RebuildBatch();
    ShowWindow(Wnd, SW_SHOWNOACTIVATE);
  }

  ~Host() {
    Render->Cmd.Synchronize();
    // Surface 内部按 DirectWrite → RTV → SwapChain 的逆依赖顺序释放资源。
    Surface.Shutdown();
    DepthTarget.ResetBuffer();
    Batch.reset();
    if (Wnd && IsWindow(Wnd)) {
      DetachWindow();
      DestroyWindow(Wnd);
    }
  }

  ui::PanelGroupNode *GetGroup() const { return Group; }

  ui::BaseNode *GetRoot() const { return Root; }

  bool IsClosePending() const { return ClosePending; }

  void RebuildBatch() {
    if (!IsRootAlive())
      return;
    Batch = std::make_unique<DX12RenderBatch>(Render, true);
    Batch->Init(Render->App->Layout.GetSubtreeUO(*Root));
  }

  void SynchronizeWindow() {
    if (!IsRootAlive())
      return;
    const auto *state = Render->App->Layout.Dock.GetState(*Root);
    if (!state || state->Placement != ui::PanelPlacement::Floating)
      return;
    ui::DockRect rect = state->FloatingRect;
    if (Render->App->Layout.Dock.Drag.Panel == Root &&
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
    if (!IsRootAlive() || !Batch)
      return;
    Resize();
    Batch->Update();
    auto constants = Render->GlobalConst;
    constants.ScreenSize = {static_cast<float>(Width),
                            static_cast<float>(Height)};
    constants.UIOrigin = {Root->Left, Root->Top};
    constants.WriteToBatch(*Batch);
  }

  void Draw() {
    if (!IsRootAlive() || !Batch)
      return;
    Ok(Render->Cmd.Allocator->Reset());
    Render->Cmd.Reset();
    Surface.PrepareFrame();
    Surface.ColorTarget.ClearBuffer();
    if (Scene && Scene->EffectiveVisible) {
      const auto &sceneViewport = Scene->Viewport();
      const auto left = static_cast<LONG>((std::max)(
          0.0f, sceneViewport.Left - Root->Left));
      const auto top = static_cast<LONG>((std::max)(
          0.0f, sceneViewport.Top - Root->Top));
      const auto right = static_cast<LONG>((std::min)(
          static_cast<float>(Width),
          sceneViewport.Left + sceneViewport.Width - Root->Left));
      const auto bottom = static_cast<LONG>((std::min)(
          static_cast<float>(Height),
          sceneViewport.Top + sceneViewport.Height - Root->Top));
      if (right > left && bottom > top) {
        // Scene 的布局仍保存在主 Layout 坐标系；减去 Floating root 原点后
        // 得到本地 viewport，使 3D 与同一子树的标题/文字精确对齐。
        const D3D12_VIEWPORT viewport{
            static_cast<float>(left), static_cast<float>(top),
            static_cast<float>(right - left), static_cast<float>(bottom - top),
            0.0f, 1.0f};
        const D3D12_RECT scissor{left, top, right, bottom};
        Render->Cmd.List->RSSetViewports(1, &viewport);
        Render->Cmd.List->RSSetScissorRects(1, &scissor);
        DepthTarget.ClearBuffer();
        Surface.ColorTarget.Bind(&DepthTarget.Dpt);
        Render->RootSignature.Bind();
        Render->GOBatch.Draw();
      }
      Surface.ApplyViewport();
    }
    Surface.ColorTarget.Bind();
    Render->RootSignature.Bind();
    Batch->Draw();
    Surface.ColorTarget.Resolve();
    Render->Cmd.CloseAndExecute();
    Render->Cmd.Synchronize();
    Surface.DrawTextAndPresent(Render->App->Layout.GetSubtreeTexts(*Root),
                               Root->Left, Root->Top);
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
  std::unordered_set<ui::BaseNode *> alive;
  for (auto *node : floating)
    alive.insert(node);

  bool closesTree = false;
  for (const auto &host : Hosts)
    if (auto *group = host->GetGroup(); host->IsClosePending() && group &&
                                        alive.contains(group)) {
      group->CloseRequested = true;
      closesTree = true;
    }
  if (closesTree) {
    Render->App->Layout.RebuildIndex();
    return Reconcile(true);
  }

  std::erase_if(Hosts, [&alive](const auto &host) {
    return !alive.contains(host->GetRoot());
  });
  for (auto *root : alive) {
    const bool exists = std::ranges::any_of(
        Hosts, [root](const auto &host) { return host->GetRoot() == root; });
    if (!exists)
      Hosts.push_back(std::make_unique<Host>(*Render, *root));
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
