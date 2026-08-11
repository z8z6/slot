#pragma once

#include <windows.h>

namespace z8 {

/**
 * 事件目标对一次输入的处理结果。
 *
 * Capture 除了阻止继续路由，还要求后续拖动与抬起回到同一目标；把该语义放在
 * Core 层后，场景 Object、UI 节点与可组合 Behavior 可以共享同一事件契约。
 */
enum class EventReply { Ignored, Handled, Capture };

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
 *
 * 该类型只描述平台输入，不包含命中测试、冒泡或捕获存储；这些策略分别由场景
 * 和 Layout 路由器决定，避免 Core 反向依赖具体 UI 实现。
 */
class EventTarget {
public:
  virtual ~EventTarget() = default;

  /** 指针按下入口；Capture 表示开始一段由路由器维持的独占手势。 */
  virtual EventReply OnMouseDown(MouseMovArgs) {
    return EventReply::Ignored;
  }
  /** 无捕获移动入口，用于悬停反馈或场景观察控制。 */
  virtual EventReply OnMouseMove(MouseMovArgs) {
    return EventReply::Ignored;
  }
  /** 捕获期间的移动入口，参数增量相对上一条窗口指针消息。 */
  virtual EventReply OnMouseDrag(MouseMovArgs) {
    return EventReply::Ignored;
  }
  /** 指针释放入口，目标应在此结束当前手势状态。 */
  virtual EventReply OnMouseUp(MouseMovArgs) { return EventReply::Ignored; }
  /** 滚轮入口；坐标已由平台层转换到窗口客户区。 */
  virtual EventReply OnMouseWheel(MouseWheelArgs) {
    return EventReply::Ignored;
  }
  /** 键盘按下与释放入口；默认不消费，以便路由器继续分发。 */
  virtual EventReply OnKeyDown(KeyArgs) { return EventReply::Ignored; }
  virtual EventReply OnKeyUp(KeyArgs) { return EventReply::Ignored; }
};

} // namespace z8
