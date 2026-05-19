//
// Created by zhou_zhengming on 2026/5/15.
//

#include "UI/Object/Camera.h"

using namespace DirectX;
using namespace z8;

Camera::Camera()
:
Target(0,0,0),
Up(0,1,0)
{
  Transform.Position.z = -20;
  UpdateView();
}

void Camera::UpdateView()
{
  XMVECTOR pos = XMVectorSet(Transform.Position.x, Transform.Position.y, Transform.Position.z, 1.0f);
  XMVECTOR target = XMLoadFloat3(&Target);
  XMVECTOR up = XMLoadFloat3(&Up);

  // pos 和 target 不能重叠

  XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
  XMStoreFloat4x4(&View, view);
}

void Camera::UpdateProj(float aspect)
{
  XMMATRIX P = XMMatrixPerspectiveFovLH(XMConvertToRadians(Fov), aspect, Near, Far);
  XMStoreFloat4x4(&Proj, P);
}

void Camera::UpdateTarget() {
  // 欧拉角转弧度
  float yaw   = XMConvertToRadians(Transform.Rotation.y);
  float pitch = XMConvertToRadians(Transform.Rotation.x);

  // 计算相机前方向向量
  Target = {
    cosf(pitch) * sinf(yaw),
    sinf(pitch),
    cosf(pitch) * cosf(yaw)
  };
}
