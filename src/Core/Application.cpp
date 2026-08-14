//
// Created by zhou_zhengming on 2026/5/7.
//

#include "Core/Application.h"
#include "Target/Render.h"
#include <WindowsX.h>
#include <ostream>
#include <utility>

#include "Core/Event.h"
#include "Core/ScenePicker.h"
#include "Light/Light.h"
#include "Light/ParallelLight.h"
#include "Object/Camera/Camera.h"
#include "Object/GameObject/RotateCube.h"
#include "Object/Object.h"
#include "Phys/Collider.h"
#include "UI/Declarative/XamlHotReload.h"
#include "UI/Layout/SceneNode.h"

#include <dwmapi.h>

using namespace z8;
using namespace std;

z8::Application::Application() {
  // Application 自身作为事件目标保留 UI→Scene 回退策略；平台层只负责把
  // HWND 消息翻译为共享 EventTarget 协议。
  AttachWindow(Window.Wnd, *this);
  Application::Apps.push_back(this);
}

Application::~Application() {
  // 析构函数体执行时所有成员仍存活；显式关闭 Render 可避免成员逆序析构导致
  // Resources/Scene 先于 GPU 缓存失效，也保证 D3D11On12 在 Window 之前退出。
  DetachWindow();
  if (Render) {
    Render->Shutdown();
    Render.reset();
  }
  // 从 Apps 中移除，防止退出消息循环后继续访问正在析构的实例。
  std::erase(Apps, this);
}

void Application::Init() {
  ActiveScene.SetCamera(std::make_unique<Camera>());
  ActiveScene.SetLight(std::make_unique<ParallelLight>());
  PrepareScene();
  Render = Render::CreateRender(this);
  Render->Init();
  Window.Open();
}

void Application::PrepareScene() { ActiveScene.CreateGameObject<RotateCube>(); }

int z8::Application::Run() {
  MSG msg = {nullptr};

  while (msg.message != WM_QUIT) {
    // 一次排空当前消息批次后仍绘制一帧。拖拽会连续产生 WM_MOUSEMOVE；若把
    // 绘制放在“没有消息”的分支中，Terminal 虽已收到日志却会一直没有呈现机会。
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
      if (msg.message == WM_QUIT)
        break;
    }
    if (msg.message == WM_QUIT)
      break;
    for (auto *App : Apps) {
      App->Timer.Tick();
      App->ShowFrame();
      if (App->XamlReload) {
        const auto status = App->XamlReload->Poll(App->Layout);
        if (status == ui::XamlReloadStatus::Reloaded) {
          App->OnLayoutReloaded();
          App->Layout.WriteTerminal("[XAML] Reloaded: " +
                                    App->XamlReload->GetPath().string());
        } else if (status == ui::XamlReloadStatus::Failed)
          App->Layout.WriteTerminal("[XAML] Reload failed: " +
                                    App->XamlReload->GetLastError());
      }
      App->OnFrame();
      App->Layout.Calculate(App->Window.LogicalWidth(),
                            App->Window.LogicalHeight());
      App->Render->Update();
      App->Render->Draw();
    }
  }

  return static_cast<int>(msg.wParam);
}

bool Application::EnableXamlHotReload(const std::string &fileName) {
  auto reload = std::make_unique<ui::XamlHotReload>(fileName);
  if (reload->Poll(Layout) != ui::XamlReloadStatus::Reloaded)
    return false;
  XamlReload = std::move(reload);
  OnLayoutReloaded();
  return true;
}

void Application::SelectSceneObject(GameObject *object) {
  if (SelectedSceneObject == object)
    return;
  SelectedSceneObject = object;
  OnSceneSelectionChanged(SelectedSceneObject);
}

LRESULT Application::HandleWindowMessage(HWND Wnd, UINT Msg, WPARAM wParam,
                                         LPARAM lParam) {
  switch (Msg) {
    // WM_ACTIVATE is sent when the window is activated or deactivated.
    // We pause the game when the window is deactivated and unpause it
    // when it becomes active.
  case WM_ACTIVATE:
    return 0;

  case WM_NCHITTEST: {
    const LRESULT nativeHit = DefWindowProcW(Wnd, Msg, wParam, lParam);
    if (nativeHit != HTCLIENT || IsZoomed(Wnd))
      return nativeHit;

    POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    ScreenToClient(Wnd, &point);
    RECT client{};
    GetClientRect(Wnd, &client);
    const int resizeInset = (std::max)(1, MulDiv(6, GetDpiForWindow(Wnd), 96));
    const bool left = point.x < resizeInset;
    const bool right = point.x >= client.right - resizeInset;
    const bool top = point.y < resizeInset;
    const bool bottom = point.y >= client.bottom - resizeInset;

    // DWM 主题下可见窗口边框很薄，指针容易落入客户区并被最外层 Panel
    // 捕获。把紧邻外框的客户区明确交还给 Win32，稳定触发系统尺寸循环。
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
    return nativeHit;
  }

    // WM_SIZE is sent when the user resizes the window.
  case WM_SIZE: {
    Window.Width = LOWORD(lParam);
    Window.Height = HIWORD(lParam);
    if (wParam == SIZE_MINIMIZED)
      return 0;
    Layout.Calculate(Window.LogicalWidth(), Window.LogicalHeight());
    if (Render) {
      Render->Resize();
      if (InSizeMove) {
        // Win32 的尺寸循环会暂停外层 PeekMessage 帧循环。必须在 WM_SIZE 内
        // 提交新尺寸帧，否则 DWM 只能拉伸旧交换链，文字和图形会同步变形。
        Timer.Tick();
        Render->Update();
        Render->Draw();
        // Present(0) 只把帧排入合成队列；尺寸循环若立即推进到下一个窗口矩形，
        // 右侧或底侧可能短暂暴露尚未合成的区域。等待 DWM 消费本帧后再返回。
        DwmFlush();
      }
    }
    return 0;
  }

  case WM_DPICHANGED: {
    Window.UpdateDpi(HIWORD(wParam));
    const auto *suggested = reinterpret_cast<const RECT *>(lParam);
    // PMv2 不会替应用缩放客户内容；采用系统建议的物理窗口矩形后，随后的
    // WM_SIZE 会按新 DPI 重建交换链，而 Layout 继续看到稳定的逻辑尺寸。
    SetWindowPos(Wnd, nullptr, suggested->left, suggested->top,
                 suggested->right - suggested->left,
                 suggested->bottom - suggested->top,
                 SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
    return 0;
  }

  case WM_MOVE: {
    return 0;
  }

  case WM_MOVING: {
    return TRUE;
  }

  case WM_SIZING: {
    return TRUE;
  }
    // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
  case WM_ENTERSIZEMOVE:
    InSizeMove = true;
    return 0;

    // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
    // Here we reset everything based on the new window dimensions.
  case WM_EXITSIZEMOVE:
    InSizeMove = false;
    Layout.Calculate(Window.LogicalWidth(), Window.LogicalHeight());
    if (Render)
      Render->Resize();
    return 0;

  case WM_CLOSE:
    // WM_CLOSE 只是关闭请求；显式销毁 HWND 才会产生 WM_DESTROY 并结束消息循环。
    DestroyWindow(Wnd);
    return 0;

    // 关闭窗口
  case WM_DESTROY: {
    if (!--Window::AliveCount)
      PostQuitMessage(0);
    return 0;
  }
    // Catch this message so to prevent the window from becoming too small.
  case WM_GETMINMAXINFO:
    reinterpret_cast<MINMAXINFO *>(lParam)->ptMinTrackSize.x =
        MulDiv(200, static_cast<int>(Window.Dpi), USER_DEFAULT_SCREEN_DPI);
    reinterpret_cast<MINMAXINFO *>(lParam)->ptMinTrackSize.y =
        MulDiv(200, static_cast<int>(Window.Dpi), USER_DEFAULT_SCREEN_DPI);
    return 0;

  }
  LRESULT result = 0;
  if (DispatchInputMessage(Wnd, Msg, wParam, lParam, result))
    return result;
  return DefWindowProcW(Wnd, Msg, wParam, lParam);
}

template <typename Handler> void Application::ForEachObject(Handler &&handler) {
  // 键盘和窗口事件没有指针目标，仍按对象集合广播。
  for (auto *object : Layout.GetUO())
    handler(*object);
  ForEachSceneObject(std::forward<Handler>(handler));
}

template <typename Handler>
void Application::ForEachSceneObject(Handler &&handler) {
  for (auto *object : ActiveScene.GetGameObjects())
    handler(*object);
  if (auto *camera = ActiveScene.GetCamera())
    handler(*camera);
  if (auto *light = ActiveScene.GetLight())
    handler(*light);
}

void z8::Application::ShowFrame() const {
  static int Frames = 0;
  static float timeElapsed = 0.0f;

  ++Frames;

  // 累计 1s 计算一次
  if (Timer.TimeTotal - timeElapsed >= 1.0f) {
    // fps = frameCnt / 1
    int fps = Frames;
    float mspf = 1000.0f / static_cast<float>(fps);

    wstring FpsText = Window.Caption + L"    fps: " + to_wstring(fps) +
                      L"   mspf: " + to_wstring(mspf);

    SetWindowTextW(Window.Wnd, FpsText.c_str());

    // 重置
    Frames = 0;
    timeElapsed += 1.0f;
  }
}

MouseCursor Application::GetMouseCursor(MouseMovArgs args) const {
  return Layout.GetMouseCursor(args);
}

EventReply Application::OnMouseMove(MouseMovArgs args) {
  const auto reply = Layout.OnMouseMove(args);
  if (reply != EventReply::Ignored)
    return reply;
  ForEachSceneObject([&](Object &object) { object.OnMouseMove(args); });
  return EventReply::Handled;
}

EventReply Application::OnMouseDrag(MouseMovArgs args) {
  const auto reply = Layout.OnMouseDrag(args);
  if (reply != EventReply::Ignored)
    return reply;
  ForEachSceneObject([&](Object &object) {
    // 场景对象原有协议在拖动时同时接收位置更新和 Drag，保持相机与物体
    // 对普通 MouseMove 的观察行为不因平台层重构而改变。
    object.OnMouseMove(args);
    object.OnMouseDrag(args);
  });
  return EventReply::Handled;
}

EventReply Application::OnMouseDown(MouseMovArgs args) {
  // 命中 UI 则返回
  const auto reply = Layout.OnMouseDown(args);
  if (reply != EventReply::Ignored)
    return reply;
  if (args.Button == MouseButton::Left) {
    auto *sceneNode = Layout.GetSceneNode();
    auto *camera = ActiveScene.GetCamera();
    if (sceneNode && camera) {
      const auto &viewport = sceneNode->Viewport();
      const ScenePickRect pickRect{viewport.Left, viewport.Top, viewport.Width,
                                   viewport.Height};
      SelectSceneObject(ScenePicker::Pick(
          ActiveScene, Resources, *camera, pickRect,
          static_cast<float>(args.X), static_cast<float>(args.Y)));
    }
  }
  ForEachSceneObject([&](Object &object) { object.OnMouseDown(args); });
  return EventReply::Handled;
}

EventReply Application::OnMouseUp(MouseMovArgs args) {
  // 命中 UI 则返回
  const auto reply = Layout.OnMouseUp(args);
  if (reply != EventReply::Ignored)
    return reply;
  ForEachSceneObject([&](Object &object) { object.OnMouseUp(args); });
  return EventReply::Handled;
}

EventReply Application::OnMouseWheel(MouseWheelArgs args) {
  // SceneNode 是 UI 布局中的输入窗口；只有普通工具控件会阻止滚轮进入场景。
  const auto reply = Layout.OnMouseWheel(args);
  if (reply != EventReply::Ignored)
    return reply;
  ForEachSceneObject([&](Object &object) { object.OnMouseWheel(args); });
  return EventReply::Handled;
}

EventReply Application::OnKeyDown(KeyArgs args) {
  const auto reply = Layout.OnKeyDown(args);
  if (reply != EventReply::Ignored)
    return reply;
  ForEachObject([&](Object &object) { object.OnKeyDown(args); });
  return EventReply::Handled;
}

EventReply Application::OnKeyUp(KeyArgs args) {
  const auto reply = Layout.OnKeyUp(args);
  if (reply != EventReply::Ignored)
    return reply;
  ForEachObject([&](Object &object) { object.OnKeyUp(args); });
  return EventReply::Handled;
}

void Application::OnPointerCaptureLost() { Layout.OnPointerCaptureLost(); }

EventReply Application::OnTextInput(wchar_t character) {
  return Layout.OnTextInput(character);
}
