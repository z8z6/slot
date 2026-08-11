#pragma once

#include <windows.h>

namespace z8 {

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


} // namespace z8
