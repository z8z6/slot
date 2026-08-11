//
// Created by zhou_zhengming on 2026/5/7.
//

#pragma once

#include "Event.h"
#include "Resource/ResourceManager.h"
#include "Scene.h"
#include "Timer.h"
#include "UI/Layout/Layout.h"
#include "Window.h"

namespace z8 {
class Render;
class UIObject;
class Application {
public:
  Window Window;
  Render *Render = nullptr;
  Timer Timer;
  ResourceManager Resources;
  Scene ActiveScene;
  ui::Layout Layout;

  Application();
  virtual ~Application();

  virtual void Init();
  LRESULT CALLBACK MsgHandler(HWND, UINT, WPARAM, LPARAM);

  // 保存所有的 App
  inline static std::vector<Application *> Apps;
  // 所有 App 都在这个方法中处理
  static int Run();
  static LRESULT CALLBACK FakeMsgHandler(HWND, UINT, WPARAM, LPARAM);

private:
  virtual void PrepareScene();
  void ShowFrame() const;
  void OnMouseMove(MouseMovArgs);
  void OnMouseDown(MouseMovArgs);
  void OnMouseUp(MouseMovArgs);
  void OnKeyDown(KeyArgs);
  void OnKeyUp(KeyArgs);
};
} // namespace z8
