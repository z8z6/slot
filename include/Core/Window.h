//
// Created by zhou_zhengming on 2026/5/7.
//

#pragma once

#include <string>
#include <windows.h>

namespace z8 {

class Window {
public:
  HINSTANCE Inst;
  HWND Wnd;
  WNDCLASS* WndClass;

  int Width = 1000;
  int Height = 800;
  std::wstring Caption = L"window";

public:
  Window();
  void Open() const;

public:
  inline static HINSTANCE Instance;
  inline static int AliveCount;

private:
  static bool Init();
  inline static bool IsInit = Init();
  inline static WNDCLASS DefaultWndClass;
};
} // namespace z8
