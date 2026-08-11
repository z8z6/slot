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
class Object : public EventTarget {
public:
  Transform Transform;

  virtual ~Object() = default;

  /** 每帧更新入口；对象不得保存传入 Timer 的所有权。 */
  virtual void Update(Timer*) {}
};
}
