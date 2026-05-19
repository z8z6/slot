//
// Created by zhou_zhengming on 2026/5/19.
//

#pragma once
#include "Camera.h"

namespace z8 {
class FirstPersonCamera : public Camera {
public:
  const float SensitivityX = 0.20f;
  const float SensitivityY = 0.12f;

  void OnMouseMove(MouseMovArgs) override;
  void OnKeyDown(KeyArgs) override;
};
}






