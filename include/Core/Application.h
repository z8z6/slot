//
// Created by zhou_zhengming on 2026/5/7.
//

#pragma once

#include "Timer.h"
#include "Window.h"
#include "Shape/IShape.h"
#include <vector>

namespace z8
{
class IRender;
class Application {
public:
  Window Window;
  IRender* Render;
  IShape* Shape;
  Timer Timer;

  Application();

  LRESULT CALLBACK MsgHandler(HWND, UINT, WPARAM, LPARAM);

  inline static std::vector<Application*> Apps;
  // 所有 App 都在这个方法中处理
  static int Run();
  static LRESULT CALLBACK FakeMsgHandler(HWND, UINT, WPARAM, LPARAM);

private:
  void ShowFrame() const;
};
}






