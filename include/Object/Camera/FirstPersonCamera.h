//
// Created by zhou_zhengming on 2026/5/19.
//

#pragma once
#include "Camera.h"

namespace z8 {
class FirstPersonCamera : public Camera {
public:
  const float SensitivityX = 0.03f;
  const float SensitivityY = 0.08f;
  const float SpeedX = 1.0f;
  const float SpeedZ = 1.0f;
  const float SpeedY = 0.2f;

  EventReply OnMouseMove(MouseMovArgs) override;
  EventReply OnKeyDown(KeyArgs) override;
};
}






