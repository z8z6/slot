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

  // Manifest 已在进程启动时启用 PMv2；初始客户区按系统 DPI 放大，使 1440×840
  // 仍表示逻辑尺寸，而交换链直接获得显示器物理像素，不再交给 DWM 插值。
  UpdateDpi(GetDpiForSystem());
  Width = MulDiv(Width, static_cast<int>(Dpi), USER_DEFAULT_SCREEN_DPI);
  Height = MulDiv(Height, static_cast<int>(Dpi), USER_DEFAULT_SCREEN_DPI);
  RECT R = {0, 0, Width, Height};
  AdjustWindowRectExForDpi(&R, WS_OVERLAPPEDWINDOW, false, 0, Dpi);
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

float Window::LogicalHeight() const {
  return static_cast<float>(Height) / DpiScale;
}

float Window::LogicalWidth() const {
  return static_cast<float>(Width) / DpiScale;
}

void Window::UpdateDpi(unsigned dpi) {
  // 防御异常消息值，保证物理/逻辑坐标换算永远可逆且不会除零。
  Dpi = dpi ? dpi : USER_DEFAULT_SCREEN_DPI;
  DpiScale = static_cast<float>(Dpi) / USER_DEFAULT_SCREEN_DPI;
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
