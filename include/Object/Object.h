//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once
#include "Transform.h"
#include "Core/Event.h"

namespace z8
{
class Camera;
class Timer;
/**
 * 物体基类
 */
class Object {
public:
  Transform Transform;

  virtual ~Object() = default;

  /** 每帧更新入口；对象不得保存传入 Timer 的所有权。 */
  virtual void Update(Timer*) {}
  /** 鼠标事件默认不处理，派生对象按需覆写。 */
  virtual void OnMouseUp(MouseMovArgs) {}
  virtual void OnMouseMove(MouseMovArgs) {}
  /** 鼠标被任一按键捕获并移动时触发，适合实现窗口或控件拖拽。 */
  virtual void OnMouseDrag(MouseMovArgs) {}
  virtual void OnMouseDown(MouseMovArgs) {}
  /** 键盘事件默认不处理，参数包含按键重复和扫描码信息。 */
  virtual void OnKeyUp(KeyArgs) {}
  virtual void OnKeyDown(KeyArgs) {}
};
}

