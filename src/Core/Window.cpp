//
// Created by zhou_zhengming on 2026/5/7.
//

#include "Core/Window.h"
#include <cassert>
#include <winuser.h>

using namespace z8;

z8::Window::Window() {
  Inst = Window::Instance;
  WndClass = &DefaultWndClass;

  RECT R = {0, 0, Width, Height};
  AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
  int width = R.right - R.left;
  int height = R.bottom - R.top;

  Wnd = CreateWindow(WndClass->lpszClassName,
                     reinterpret_cast<LPCSTR>(Caption.c_str()),
                     WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width,
                     height, nullptr, nullptr, Inst, nullptr);
  assert(Wnd);
  ++AliveCount;
}

void Window::Open() const {
  ShowWindow(Wnd, SW_SHOW);
  UpdateWindow(Wnd);
}

int Window::Run() {
  MSG msg = {nullptr};

  while (msg.message != WM_QUIT) {
    // If there are Window messages then process them.
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return static_cast<int>(msg.wParam);
}

LRESULT Window::DefaultMsgHandler(HWND Wnd, UINT Msg, WPARAM wParam,
                                  LPARAM lParam) {
  switch (Msg) {
  // WM_ACTIVATE is sent when the window is activated or deactivated.
    // We pause the game when the window is deactivated and unpause it
    // when it becomes active.
  case WM_ACTIVATE:

    return 0;

    // WM_SIZE is sent when the user resizes the window.
  case WM_SIZE:

    return 0;

    // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
  case WM_ENTERSIZEMOVE:

    return 0;

    // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
    // Here we reset everything based on the new window dimensions.
  case WM_EXITSIZEMOVE:

    return 0;

  // 关闭窗口
  case WM_DESTROY: {
    if (!--AliveCount)
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
    ((MINMAXINFO *)lParam)->ptMinTrackSize.x = 200;
    ((MINMAXINFO *)lParam)->ptMinTrackSize.y = 200;
    return 0;

  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:

    return 0;
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP:
    return 0;
  case WM_MOUSEMOVE:

    return 0;
  case WM_KEYUP:

    return 0;
  default:
    return DefWindowProc(Wnd, Msg, wParam, lParam);
  }
}

bool Window::Init() {
  DefaultWndClass.style = CS_HREDRAW | CS_VREDRAW;
  DefaultWndClass.lpfnWndProc = DefaultMsgHandler;
  DefaultWndClass.cbClsExtra = 0;
  DefaultWndClass.cbWndExtra = 0;
  DefaultWndClass.hInstance = Window::Instance;
  DefaultWndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  DefaultWndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
  DefaultWndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
  DefaultWndClass.lpszMenuName = nullptr;
  DefaultWndClass.lpszClassName = reinterpret_cast<LPCSTR>(L"DefaultWindowClass");

  assert(RegisterClass(&DefaultWndClass));
  return true;
}
