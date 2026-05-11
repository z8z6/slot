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

  Wnd = CreateWindowW(WndClass->lpszClassName,
                     Caption.c_str(),
                     WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width,
                     height, nullptr, nullptr, Inst, nullptr);
  assert(Wnd);
  ++AliveCount;
}

void Window::Open() const {
  ShowWindow(Wnd, SW_SHOW);
  UpdateWindow(Wnd);
}

float Window::AspectRatio() const {
  return static_cast<float>(Width) / Height;
}

bool Window::Init() {
  DefaultWndClass.style = CS_HREDRAW | CS_VREDRAW;
  DefaultWndClass.lpfnWndProc = DefWindowProcW;
  DefaultWndClass.cbClsExtra = 0;
  DefaultWndClass.cbWndExtra = 0;
  DefaultWndClass.hInstance = Window::Instance;
  DefaultWndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  DefaultWndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
  DefaultWndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
  DefaultWndClass.lpszMenuName = nullptr;
  DefaultWndClass.lpszClassName = L"DefaultWindowClass";

  auto R = RegisterClassW(&DefaultWndClass);
  assert(R);
  return true;
}
