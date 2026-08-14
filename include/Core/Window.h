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
  WNDCLASSW* WndClass;

  int Width = 1440;
  int Height = 840;
  unsigned Dpi = USER_DEFAULT_SCREEN_DPI;
  float DpiScale = 1.0f;
  std::wstring Caption = L"window";

public:
  Window();
  float AspectRatio() const;
  float LogicalHeight() const;
  float LogicalWidth() const;
  void Open() const;
  /** 更新窗口物理 DPI；Layout 始终通过 LogicalWidth/Height 消费 96 DPI 坐标。 */
  void UpdateDpi(unsigned dpi);

public:
  inline static HINSTANCE Instance;
  inline static int AliveCount;

private:
  static bool Init();
  inline static bool IsInit = Init();
  inline static WNDCLASSW DefaultWndClass;
};
} // namespace z8
