//
// Created by zhou_zhengming on 2026/5/19.
//

#include "UI/Object/FirstPersonCamera.h"

#include <iostream>
#include <ostream>

using namespace z8;
using namespace DirectX;

void FirstPersonCamera::OnMouseMove(MouseMovArgs Args) {
  // 鼠标偏移量
  float dx = static_cast<float>(Args.DeltaX) * SensitivityX;
  float dy = static_cast<float>(Args.DeltaY) * SensitivityY;

  Transform.Rotation.y += dx;   // Yaw 左右看
  Transform.Rotation.x -= dy;  // Pitch 上下看

  // 限制俯仰角，防止相机翻转
  if (Transform.Rotation.x > 89.0f)  Transform.Rotation.x = 89.0f;
  if (Transform.Rotation.x < -89.0f) Transform.Rotation.x = -89.0f;

  UpdateTarget();

  std::cout << Transform << std::endl;
}

void FirstPersonCamera::OnKeyDown(KeyArgs Args) {
  // 移动速度
  const float speed = 5.0f;

  // 从你的 Transform 欧拉角获取 Yaw
  float yaw = XMConvertToRadians(Transform.Rotation.y);

  // 计算相机前方向、右方向（XZ平面）
  float forwardX = sinf(yaw);
  float forwardZ = cosf(yaw);
  float rightX   = cosf(yaw);
  float rightZ   = -sinf(yaw);

  switch (Args.Key)
  {
  case 'W': // 前进
    Transform.Position.x += forwardX * speed;
    Transform.Position.z += forwardZ * speed;
    break;
  case 'S': // 后退
    Transform.Position.x -= forwardX * speed;
    Transform.Position.z -= forwardZ * speed;
    break;
  case 'A': // 左移
    Transform.Position.x -= rightX * speed;
    Transform.Position.z -= rightZ * speed;
    break;
  case 'D': // 右移
    Transform.Position.x += rightX * speed;
    Transform.Position.z += rightZ * speed;
    break;
  }
  std::cout << Transform << std::endl;
}