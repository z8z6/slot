//
// Created by zhou_zhengming on 2026/5/7.
//

#pragma once

#include "Event.h"
#include "Resource/ResourceManager.h"
#include "Scene.h"
#include "Timer.h"
#include "UI/Layout/Layout.h"
#include "Win32WindowHost.h"
#include "Window.h"

namespace z8 {
class Render;
class UIObject;
namespace ui {
class XamlHotReload;
}
class Application : public EventTarget, private Win32WindowHost {
public:
  Window Window;
  std::unique_ptr<Render> Render;
  Timer Timer;
  ResourceManager Resources;
  Scene ActiveScene;
  GameObject *SelectedSceneObject = nullptr;
  ui::Layout Layout;

  inline static std::vector<Application *> Apps;

  Application();
  ~Application() override;


  bool EnableXamlHotReload(const std::string &fileName);
  /** 统一提交树选择或视口拾取结果，并通知编辑器刷新检查器。 */
  void SelectSceneObject(GameObject *object);
  virtual void Init();
  static int Run();

private:
  // 模态尺寸循环属于主交换链策略；通用指针状态由 Win32WindowHost 保存。
  bool InSizeMove = false;
  std::unique_ptr<ui::XamlHotReload> XamlReload;

  /** 将事件发送给 UI、场景、相机和灯光中的每一个 Object。 */
  template <typename Handler> void ForEachObject(Handler &&handler);
  /** 指针未被 UI 消费时，只向 3D 场景部分继续传播。 */
  template <typename Handler> void ForEachSceneObject(Handler &&handler);

  MouseCursor GetMouseCursor(MouseMovArgs args) const override;
  LRESULT HandleWindowMessage(HWND window, UINT message, WPARAM wParam,
                              LPARAM lParam) override;
  /** 每帧布局前同步编辑器业务绑定；基础 Application 没有额外工作。 */
  virtual void OnFrame() {}
  /** XAML 成功替换控件树后重建业务绑定；失败时不触碰旧树。 */
  virtual void OnLayoutReloaded() {}
  /** 场景选择变化扩展点；基础 Application 不假定存在 Details 控件。 */
  virtual void OnSceneSelectionChanged(GameObject *) {}
  EventReply OnKeyDown(KeyArgs args) override;
  EventReply OnKeyUp(KeyArgs args) override;
  EventReply OnMouseDown(MouseMovArgs args) override;
  EventReply OnMouseDrag(MouseMovArgs args) override;
  EventReply OnMouseMove(MouseMovArgs args) override;
  EventReply OnMouseUp(MouseMovArgs args) override;
  EventReply OnMouseWheel(MouseWheelArgs args) override;
  void OnPointerCaptureLost() override;
  EventReply OnTextInput(wchar_t character) override;
  virtual void PrepareScene();
  void ShowFrame() const;
};
} // namespace z8
