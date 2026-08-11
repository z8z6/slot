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
  /** 将事件发送给 UI、场景、相机和灯光中的每一个 Object。 */
  template <typename Handler>
  void ForEachObject(Handler&& handler);
  /** 指针未被 UI 消费时，只向 3D 场景部分继续传播。 */
  template <typename Handler>
  void ForEachSceneObject(Handler&& handler);

  void OnMouseMove(MouseMovArgs);
  void OnMouseDown(MouseMovArgs);
  void OnMouseUp(MouseMovArgs);
  void OnMouseWheel(MouseWheelArgs);
  void OnKeyDown(KeyArgs);
  void OnKeyUp(KeyArgs);

  // 输入和模态尺寸循环状态必须按 Application 保存，多窗口之间不能互相污染。
  int MouseX = 0;
  int MouseY = 0;
  int WindowX = 0;
  int WindowY = 0;
  bool HasMousePosition = false;
  bool InSizeMove = false;
};
} // namespace z8
