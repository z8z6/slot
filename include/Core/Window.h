//
// Created by zhou_zhengming on 2026/5/7.
//

#pragma once

#include <string>
#include <windows.h>

namespace z8 {

class Window {
  HINSTANCE Inst;
  HWND Wnd;

  int Width = 1000;
  int Height = 800;
  std::wstring Caption = L"window";

public:
  explicit Window(HINSTANCE hInst);
  int Run();

  static LRESULT CALLBACK DefaultMsgHandler(HWND, UINT, WPARAM, LPARAM);
};
} // namespace z8
