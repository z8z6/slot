//
// Created by zhou_zhengming on 2026/5/7.
//

#pragma once

#include "Timer.h"
#include "Window.h"
#include <vector>

#include "Event.h"

namespace z8
{
class Render;
class GameObject;
class Camera;
class Application {
public:
  Window Window;
  Render* Render;
  Timer Timer;
  std::vector<GameObject*> Objects;
  Camera* Camera;

  Application();
  virtual ~Application() = default;

  virtual void Init();
  LRESULT CALLBACK MsgHandler(HWND, UINT, WPARAM, LPARAM);

  inline static std::vector<Application*> Apps;
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
}






