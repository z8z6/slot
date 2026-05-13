//
// Created by zhou_zhengming on 2026/5/13.
//

#include "UI/Object/Transform.h"

using namespace DirectX;
using namespace z8;

Transform::Transform() : Position{0,0,0}, Rotation{0,0,0}, Scale{1,1,1} {}

DirectX::XMMATRIX Transform::GetWorldMatrix() const
{
  // 顺序：缩放 → 旋转 → 平移 (DX标准变换顺序)
  XMMATRIX scale = XMMatrixScaling(Scale.x, Scale.y, Scale.z);
  XMMATRIX rot = XMMatrixRotationRollPitchYaw(Rotation.x, Rotation.y, Rotation.z);
  XMMATRIX translate = XMMatrixTranslation(Position.x, Position.y, Position.z);

  return scale * rot * translate;
}