//
// Created by zhou_zhengming on 2026/5/7.
//

#include "Core/Application.h"
#include "Target/Render.h"
#include <WindowsX.h>
#include <ostream>
#include <utility>

#include "Object/Camera/Camera.h"
#include "Object/GameObject/RotateCube.h"
#include "Core/Event.h"
#include "Light/ParallelLight.h"
#include "Light/Light.h"
#include "Object/Object.h"
#include "Phys/Collider.h"

using namespace z8;
using namespace std;

namespace {

MouseButton GetMouseButton(UINT message, WPARAM wParam) {
  switch (message) {
  case WM_LBUTTONDOWN:
  case WM_LBUTTONUP: return MouseButton::Left;
  case WM_MBUTTONDOWN:
  case WM_MBUTTONUP: return MouseButton::Middle;
  case WM_RBUTTONDOWN:
  case WM_RBUTTONUP: return MouseButton::Right;
  case WM_XBUTTONDOWN:
  case WM_XBUTTONUP:
    return GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? MouseButton::X1
                                                  : MouseButton::X2;
  default: return MouseButton::None;
  }
}
} // namespace


z8::Application::Application() : Layout(this) {
  SetWindowLongPtrW(Window.Wnd, GWLP_USERDATA,
                    reinterpret_cast<LONG_PTR>(this));
  SetWindowLongPtrW(Window.Wnd, GWLP_WNDPROC,
                    reinterpret_cast<LONG_PTR>(FakeMsgHandler));
  RECT rect{};
  if (GetWindowRect(Window.Wnd, &rect)) {
    WindowX = rect.left;
    WindowY = rect.top;
  }
  Application::Apps.push_back(this);
}

Application::~Application() {
  // 从 Apps 中移除
  std::erase(Apps, this);
  delete Render;
}

void Application::Init() {
  ActiveScene.SetCamera(std::make_unique<Camera>());
  ActiveScene.SetLight(std::make_unique<ParallelLight>());
  PrepareScene();
  Render = Render::CreateRender(this);
  Render->Init();
  Window.Open();
}

void Application::PrepareScene() {
  ActiveScene.CreateGameObject<RotateCube>();
}


int z8::Application::Run() {
  MSG msg = {nullptr};

  while (msg.message != WM_QUIT) {
    // If there are Window messages then process them.
    if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }
    else {
      for (auto* App : Apps) {
        App->Timer.Tick();
        App->ShowFrame();
        App->Layout.Update();
        App->Render->Update();
        App->Render->Draw();
      }
    }
  }

  return static_cast<int>(msg.wParam);
}

LRESULT z8::Application::FakeMsgHandler(HWND Wnd, UINT Msg, WPARAM wParam,
                                  LPARAM lParam) {
  // 取出对象指针
  auto* app = reinterpret_cast<Application *>(GetWindowLongPtrW(Wnd, GWLP_USERDATA));

  if (app && Msg != WM_NCDESTROY)
    // 转发给成员函数
    return app->MsgHandler(Wnd, Msg, wParam, lParam);

  // 默认处理
  return DefWindowProcW(Wnd, Msg, wParam, lParam);
}


LRESULT Application::MsgHandler(HWND Wnd, UINT Msg, WPARAM wParam,
                                  LPARAM lParam) {
  switch (Msg) {
    // WM_ACTIVATE is sent when the window is activated or deactivated.
    // We pause the game when the window is deactivated and unpause it
    // when it becomes active.
  case WM_ACTIVATE:

    return 0;

  case WM_SETCURSOR:
    if (LOWORD(lParam) == HTCLIENT) {
      POINT point{};
      GetCursorPos(&point);
      ScreenToClient(Wnd, &point);
      LPCWSTR cursorId = IDC_ARROW;
      switch (Layout.GetMouseCursor(point.x, point.y)) {
      case MouseCursor::SizeHorizontal: cursorId = IDC_SIZEWE; break;
      case MouseCursor::SizeVertical: cursorId = IDC_SIZENS; break;
      case MouseCursor::SizeDiagonalNorthwestSoutheast:
        cursorId = IDC_SIZENWSE;
        break;
      case MouseCursor::SizeDiagonalNortheastSouthwest:
        cursorId = IDC_SIZENESW;
        break;
      default: break;
      }
      // 使用系统 DPI 感知光标，避免自绘位图在不同缩放比例下模糊。
      SetCursor(LoadCursorW(nullptr, cursorId));
      return TRUE;
    }
    break;

    // WM_SIZE is sent when the user resizes the window.
  case WM_SIZE: {
    Window.Width = LOWORD(lParam);
    Window.Height = HIWORD(lParam);
    if (wParam == SIZE_MINIMIZED) return 0;
    Layout.Update();
    // 拖动边框时 WM_SIZE 会高频到达；退出模态循环后只重建一次 GPU 资源。
    if (!InSizeMove && Render) Render->Resize();
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
    if (RECT rect{}; GetWindowRect(Wnd, &rect)) {
      WindowX = rect.left;
      WindowY = rect.top;
    }
    Layout.Update();
    if (Render) Render->Resize();
    return 0;

    // 关闭窗口
  case WM_DESTROY: {
    if (!--Window::AliveCount)
      PostQuitMessage(0);
    return 0;
  }
    // The WM_MENUCHAR message is sent when a menu is active and the user presses
    // a key that does not correspond to any mnemonic or accelerator key.
  case WM_MENUCHAR:
    // Don't beep when we alt-enter.
    return MAKELRESULT(0, MNC_CLOSE);

    // Catch this message so to prevent the window from becoming too small.
  case WM_GETMINMAXINFO:
    reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.x = 200;
    reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.y = 200;
    return 0;

  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_XBUTTONDOWN: {
    SetCapture(Wnd);
    const int x = GET_X_LPARAM(lParam);
    const int y = GET_Y_LPARAM(lParam);
    OnMouseDown(MouseMovArgs(GET_KEYSTATE_WPARAM(wParam), lParam,
                             HasMousePosition ? x - MouseX : 0,
                             HasMousePosition ? y - MouseY : 0,
                             GetMouseButton(Msg, wParam)));
    MouseX = x;
    MouseY = y;
    HasMousePosition = true;
    return 0;
  }
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
  case WM_XBUTTONUP: {
    const int x = GET_X_LPARAM(lParam);
    const int y = GET_Y_LPARAM(lParam);
    const auto state = GET_KEYSTATE_WPARAM(wParam);
    OnMouseUp(MouseMovArgs(state, lParam,
                           HasMousePosition ? x - MouseX : 0,
                           HasMousePosition ? y - MouseY : 0,
                           GetMouseButton(Msg, wParam)));
    MouseX = x;
    MouseY = y;
    HasMousePosition = true;
    if ((state & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 |
                  MK_XBUTTON2)) == 0 && GetCapture() == Wnd)
      ReleaseCapture();
    return 0;
  }
  case WM_MOUSEMOVE: {
    const int x = GET_X_LPARAM(lParam);
    const int y = GET_Y_LPARAM(lParam);
    OnMouseMove(MouseMovArgs(wParam, lParam,
                             HasMousePosition ? x - MouseX : 0,
                             HasMousePosition ? y - MouseY : 0));
    MouseX = x;
    MouseY = y;
    HasMousePosition = true;
    return 0;
  }
  case WM_MOUSEWHEEL: {
    POINT point{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    ScreenToClient(Wnd, &point);
    OnMouseWheel({static_cast<unsigned>(GET_KEYSTATE_WPARAM(wParam)),
                  point.x, point.y, GET_WHEEL_DELTA_WPARAM(wParam)});
    return 0;
  }
  case WM_KEYUP:
    OnKeyUp(KeyArgs(wParam, lParam));
    return 0;
  case WM_KEYDOWN:
    OnKeyDown(KeyArgs(wParam, lParam));
    return 0;
  case WM_SYSKEYDOWN:
    // 禁止系统处理
    if (wParam == VK_MENU || wParam == VK_F10)
      return 0;
    OnKeyDown(KeyArgs(wParam, lParam));
    return 0;
  case WM_SYSKEYUP:
    if (wParam == VK_MENU) return 0;
    OnKeyUp(KeyArgs(wParam, lParam));
    return 0;
    // 拦截系统命令（如 Alt+Space、Alt+Enter 等）
  case WM_SYSCOMMAND:
    // 菜单激活
    if ((wParam & 0xFFF0) == SC_KEYMENU)
      return 0;
    break;
  }
  return DefWindowProcW(Wnd, Msg, wParam, lParam);
}

template <typename Handler>
void Application::ForEachObject(Handler&& handler) {
  // 键盘和窗口事件没有指针目标，仍按对象集合广播。
  for (auto* object : Layout.UOs) handler(*object);
  ForEachSceneObject(std::forward<Handler>(handler));
}

template <typename Handler>
void Application::ForEachSceneObject(Handler&& handler) {
  for (auto* object : ActiveScene.GetGameObjects()) handler(*object);
  if (auto* camera = ActiveScene.GetCamera()) handler(*camera);
  if (auto* light = ActiveScene.GetLight()) handler(*light);
}

void z8::Application::ShowFrame() const {
  static int Frames = 0;
  static float timeElapsed = 0.0f;

  ++Frames;

  // 累计 1s 计算一次
  if( Timer.TimeTotal - timeElapsed >= 1.0f )
  {
    // fps = frameCnt / 1
    int fps = Frames;
    float mspf = 1000.0f / static_cast<float>(fps);

    wstring FpsText = Window.Caption +
        L"    fps: " + to_wstring(fps) +
        L"   mspf: " + to_wstring(mspf);

    SetWindowTextW(Window.Wnd, FpsText.c_str());

    // 重置
    Frames = 0;
    timeElapsed += 1.0f;
  }
}

void Application::OnMouseMove(MouseMovArgs Args)
{
  const bool dragging = (Args.State & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON |
                                       MK_XBUTTON1 | MK_XBUTTON2)) != 0;
  if (dragging ? Layout.OnMouseDrag(Args) : Layout.OnMouseMove(Args)) return;
  ForEachSceneObject([&](Object& object) {
    object.OnMouseMove(Args);
    if (dragging) object.OnMouseDrag(Args);
  });
}

void Application::OnMouseDown(MouseMovArgs Args)
{
  // 命中 UI 则返回
  if (Layout.OnMouseDown(Args)) return;
  ForEachSceneObject([&](Object& object) { object.OnMouseDown(Args); });
}

void Application::OnMouseUp(MouseMovArgs Args)
{
  // 命中 UI 则返回
  if (Layout.OnMouseUp(Args)) return;
  ForEachSceneObject([&](Object& object) { object.OnMouseUp(Args); });
}

void Application::OnMouseWheel(MouseWheelArgs Args) {
  // 命中 UI 则返回
  Layout.OnMouseWheel(Args);
}

void Application::OnKeyDown(KeyArgs Args) {
  ForEachObject([&](Object& object) { object.OnKeyDown(Args); });
}

void Application::OnKeyUp(KeyArgs Args) {
  ForEachObject([&](Object& object) { object.OnKeyUp(Args); });
}
