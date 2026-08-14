//
// Created by zhou_zhengming on 2026/5/19.
//

#pragma once
#include "BaseCamera.h"

namespace z8 {
class FirstPersonCamera : public BaseCamera {
public:
  // 编辑器视口尚未建立按键捕获/隐藏光标协议前，默认禁止鼠标驱动相机。
  bool MouseLookEnabled = false;
  const float SensitivityX = 0.03f;
  const float SensitivityY = 0.08f;
  const float SpeedX = 1.0f;
  const float SpeedZ = 1.0f;
  const float SpeedY = 0.2f;

  EventReply OnMouseMove(MouseMovArgs) override;
  EventReply OnKeyDown(KeyArgs) override;
};
}






