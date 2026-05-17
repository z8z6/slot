//
// Created by zhou_zhengming on 2026/5/7.
//

#include "Core/Application.h"
#include "UI/Object/RectObject.h"
#include "Target/Render.h"
#include <WindowsX.h>


#include <iostream>
#include <ostream>

#include "Core/Event.h"
#include "UI/Object/Camera.h"
#include "UI/Object/RotateCube.h"
#include "UI/Phys/Collider.h"

using namespace z8;
using namespace std;

z8::Application::Application() {
  Camera = new z8::Camera();
  //Objects.push_back(new RectObject());
  Objects.push_back(new RotateCube());
  Render = Render::CreateRender(this);
  Render->Init();
  SetWindowLongPtrW(Window.Wnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  SetWindowLongPtrW(Window.Wnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(FakeMsgHandler));
  Window.Open();
  Application::Apps.push_back(this);
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

LRESULT z8::Application::MsgHandler(HWND Wnd, UINT Msg, WPARAM wParam,
                                  LPARAM lParam) {
  switch (Msg) {
    // WM_ACTIVATE is sent when the window is activated or deactivated.
    // We pause the game when the window is deactivated and unpause it
    // when it becomes active.
  case WM_ACTIVATE:

    return 0;

    // WM_SIZE is sent when the user resizes the window.
  case WM_SIZE:
    Window.Width = LOWORD(lParam);
    Window.Height = HIWORD(lParam);
    std::cerr << Window.Width << " | " << Window.Height << std::endl;
    if(wParam == SIZE_MINIMIZED) return 0;
    Render->Resize();
    return 0;

    // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
  case WM_ENTERSIZEMOVE:

    return 0;

    // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
    // Here we reset everything based on the new window dimensions.
  case WM_EXITSIZEMOVE:
    Render->Resize();
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
    OnMouseDown(ButtonEventArgs(wParam, lParam));
    return 0;
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
    OnMouseUp(ButtonEventArgs(wParam, lParam));
    return 0;
  case WM_MOUSEMOVE:
    OnMouseMove(ButtonEventArgs(wParam, lParam));
    return 0;

  case WM_KEYUP:
    return 0;
  default:
    return DefWindowProcW(Wnd, Msg, wParam, lParam);
  }
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

void Application::OnMouseMove(ButtonEventArgs Args)
{
  for (auto* O : Objects)
    O->OnMouseMove(Args);
}

void Application::OnMouseDown(ButtonEventArgs Args)
{
  for (auto* O : Objects)
    O->OnMouseDown(Args);
}

void Application::OnMouseUp(ButtonEventArgs Args)
{
  for (auto* O : Objects)
    O->OnMouseUp(Args);
}
