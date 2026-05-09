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

bool Window::Init() {
  DefaultWndClass.style = CS_HREDRAW | CS_VREDRAW;
  DefaultWndClass.lpfnWndProc = DefWindowProc;
  DefaultWndClass.cbClsExtra = 0;
  DefaultWndClass.cbWndExtra = 0;
  DefaultWndClass.hInstance = Window::Instance;
  DefaultWndClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  DefaultWndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
  DefaultWndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
  DefaultWndClass.lpszMenuName = nullptr;
  DefaultWndClass.lpszClassName = reinterpret_cast<LPCSTR>(L"DefaultWindowClass");

  auto R = RegisterClass(&DefaultWndClass);
  assert(R);
  return true;
}
