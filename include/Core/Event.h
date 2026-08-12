#pragma once

#include <windows.h>

namespace z8 {

/**
 * 事件目标对一次输入的处理结果。
 *
 * Capture 除了阻止继续路由，还要求后续拖动与抬起回到同一目标；把该语义放在
 * Core 层后，场景 Object、UI 节点与可组合 Behavior 可以共享同一事件契约。
 */
enum class EventReply {
  Ignored,  // 未处理
  Handled,  // 已处理
  Capture   // 捕获
};

/** 鼠标按键；None 用于不由单一按键触发的移动事件。 */
enum class MouseButton { None, Left, Middle, Right, X1, X2 };

/** 平台无关的 UI 指针形状，由窗口适配层映射为系统光标资源。 */
enum class MouseCursor {
  Arrow,
  SizeHorizontal,
  SizeVertical,
  SizeDiagonalNorthwestSoutheast,
  SizeDiagonalNortheastSouthwest
};

/**
 * 鼠标事件参数。
 *
 * 坐标始终位于窗口客户区；位移由 Application 按窗口分别维护，避免多个窗口
 * 共用静态坐标后产生跳变。Button 在按下/抬起事件中表示发生变化的按键。
 */
struct MouseMovArgs {
  unsigned State = 0;
  int X = 0;
  int Y = 0;
  int DeltaX = 0;
  int DeltaY = 0;
  MouseButton Button = MouseButton::None;

  MouseMovArgs() = default;
  MouseMovArgs(WPARAM wParam, LPARAM lParam, int deltaX = 0,
               int deltaY = 0, MouseButton button = MouseButton::None);
};

/** 鼠标滚轮参数；坐标在进入 UI 分发前统一转换为客户区坐标。 */
struct MouseWheelArgs {
  unsigned State = 0;
  int X = 0;
  int Y = 0;
  int Delta = 0;
};

/** 键盘事件参数，保留 Win32 重复、扫描码和扩展键语义供控件判断。 */
struct KeyArgs {
  unsigned Key = 0;
  unsigned RepeatCount = 0;
  unsigned ScanCode = 0;
  bool IsExtended = false;
  bool WasDown = false;

  explicit KeyArgs(WPARAM wParam, LPARAM lParam = 0);
};

/**
 * 场景对象与 UI 节点共享的输入事件目标。
 */
class EventTarget {
public:
  virtual ~EventTarget() = default;
  virtual EventReply OnMouseDown(MouseMovArgs) {
    return EventReply::Ignored;
  }
  virtual EventReply OnMouseMove(MouseMovArgs) {
    return EventReply::Ignored;
  }
  virtual EventReply OnMouseDrag(MouseMovArgs) {
    return EventReply::Ignored;
  }
  virtual EventReply OnMouseUp(MouseMovArgs) { return EventReply::Ignored; }
  virtual EventReply OnMouseWheel(MouseWheelArgs) {
    return EventReply::Ignored;
  }
  virtual EventReply OnKeyDown(KeyArgs) { return EventReply::Ignored; }
  virtual EventReply OnKeyUp(KeyArgs) { return EventReply::Ignored; }
  virtual MouseCursor GetMouseCursor(MouseMovArgs) const {
    return MouseCursor::Arrow;
  }
  virtual void OnPointerCaptureLost() {}
};

} // namespace z8
