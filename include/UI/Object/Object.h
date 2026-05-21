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
class Object {
public:
  Transform Transform;

  virtual ~Object() = default;

  virtual void Update(Camera*, Timer*) {}
  virtual void OnMouseUp(MouseMovArgs) {}
  virtual void OnMouseMove(MouseMovArgs) {}
  virtual void OnMouseDown(MouseMovArgs) {}
  virtual void OnKeyUp(KeyArgs) {}
  virtual void OnKeyDown(KeyArgs) {}
};
}

