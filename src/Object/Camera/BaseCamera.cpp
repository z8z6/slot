//
// Created by zhou_zhengming on 2026/5/15.
//

#include "Object/Camera/BaseCamera.h"

#include <utility>

using namespace DirectX;
using namespace z8;

BaseCamera::BaseCamera()
:
Target(0,0,0),
Up(0,1,0)
{
  Transform.Position.y = 2;
  Transform.Position.z = -40;
  UpdateView();
}

void BaseCamera::Update(Timer *) {
  UpdateView();
  UpdateViewProj();
}

void BaseCamera::UpdateView()
{
  XMVECTOR pos = XMVectorSet(Transform.Position.x, Transform.Position.y, Transform.Position.z, 1.0f);
  XMVECTOR target = XMLoadFloat3(&Target);
  XMVECTOR up = XMLoadFloat3(&Up);

  // pos 和 target 不能重叠

  XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
  XMStoreFloat4x4(&View, view);
}

void BaseCamera::UpdateProj(float aspect)
{
  XMMATRIX P = XMMatrixPerspectiveFovLH(XMConvertToRadians(Fov), aspect, Near, Far);
  XMStoreFloat4x4(&Proj, P);
}

void BaseCamera::UpdateViewProj() {
  XMMATRIX view = XMLoadFloat4x4(&View);
  XMMATRIX proj = XMLoadFloat4x4(&Proj);
  XMMATRIX vp = view * proj;
  XMStoreFloat4x4(&ViewProj, vp);
}

void BaseCamera::UpdateTarget() {
  // 欧拉角转弧度
  float yaw   = XMConvertToRadians(Transform.Rotation.y);
  float pitch = XMConvertToRadians(Transform.Rotation.x);

  // 计算相机前方向向量
  XMVECTOR forward = {
    cosf(pitch) * sinf(yaw),
    sinf(pitch),
    cosf(pitch) * cosf(yaw),
    0
  };

  XMVECTOR pos = XMLoadFloat3(&Transform.Position);
  XMVECTOR targetPoint = XMVectorAdd(pos, forward);   // 真实目标点
  XMStoreFloat3(&Target, targetPoint);
}
