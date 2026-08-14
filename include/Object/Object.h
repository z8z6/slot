//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once
#include "Transform.h"
#include "Core/Event.h"

namespace z8
{
class BaseCamera;
class Timer;

class Object : public EventTarget {
public:
  Transform Transform;

  virtual void Update(Timer*) {}
};
}
