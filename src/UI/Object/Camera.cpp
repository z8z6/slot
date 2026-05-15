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
  Transform.Position.z = -10;
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

void Camera::OnMouseMove(ButtonEventArgs button_event_args)
{

}
