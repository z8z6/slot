//
// Created by zhou_zhengming on 2026/5/19.
//

#include "Object/Camera/FirstPersonCamera.h"
#include "Util/Math.h"
#include <ostream>

using namespace z8;
using namespace DirectX;

void FirstPersonCamera::OnMouseMove(MouseMovArgs Args) {
  if (GetAsyncKeyState(VK_MENU) & 0x8000) return;
  // 鼠标偏移量
  float dx = static_cast<float>(Args.DeltaX) * SensitivityX;
  float dy = static_cast<float>(Args.DeltaY) * SensitivityY;

  Transform.Rotation.y += dx;   // Yaw 左右看
  Transform.Rotation.x -= dy;  // Pitch 上下看

  // 限制俯仰角，防止相机翻转
  // @todo 旋转到边界，隐藏光标，并在每一帧将鼠标重新定位到窗口中心
  if (Transform.Rotation.x > 89.0f)  Transform.Rotation.x = 89.0f;
  if (Transform.Rotation.x < -89.0f) Transform.Rotation.x = -89.0f;

  UpdateTarget();
}

void FirstPersonCamera::OnKeyDown(KeyArgs Args) {

  // 从欧拉角获取 Yaw
  float yaw = XMConvertToRadians(Transform.Rotation.y);

  // 计算相机前方向、右方向（XZ平面）
  float forwardX = sinf(yaw);
  float forwardZ = cosf(yaw);
  float rightX   = cosf(yaw);
  float rightZ   = -sinf(yaw);

  switch (Args.Key)
  {
  case 'W': // 前进
    Transform.Position.x += forwardX * SpeedX;
    Transform.Position.z += forwardZ * SpeedZ;
    break;
  case 'S': // 后退
    Transform.Position.x -= forwardX * SpeedX;
    Transform.Position.z -= forwardZ * SpeedZ;
    break;
  case 'A': // 左移
    Transform.Position.x -= rightX * SpeedX;
    Transform.Position.z -= rightZ * SpeedZ;
    break;
  case 'D': // 右移
    Transform.Position.x += rightX * SpeedX;
    Transform.Position.z += rightZ * SpeedZ;
    break;
  case VK_SPACE:
    Transform.Position.y += SpeedY;
    break;
  // @todo 长按失效
  case VK_SHIFT:
    Transform.Position.y -= SpeedY;
    break;
  }
  UpdateTarget();
}