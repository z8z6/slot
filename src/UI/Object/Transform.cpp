//
// Created by zhou_zhengming on 2026/5/13.
//

#include "UI/Object/Transform.h"
#include "Util/Math.h"
#include <ostream>

using namespace DirectX;
using namespace z8;

Transform::Transform()
    : Position{0, 0, 0}, Rotation{0, 0, 0}, Scale{1, 1, 1}, Radius(0.0f),
      Theta(0.0f), Phi(0.0f), World(Math::Identity4x4),
      WorldViewProj(Math::Identity4x4) {}

void Transform::UpdateWorld() {
  // 顺序：缩放 -> 旋转 -> 平移 (DX标准变换顺序)
  // 统一使用齐次坐标进行计算

  // 缩放矩阵
  XMMATRIX scale = XMMatrixScaling(Scale.x, Scale.y, Scale.z);
  // 旋转矩阵
  XMMATRIX rot =
      XMMatrixRotationRollPitchYaw(Rotation.x, Rotation.y, Rotation.z);
  // 偏移矩阵
  XMMATRIX translate = XMMatrixTranslation(Position.x, Position.y, Position.z);

  XMMATRIX world = scale * rot * translate;
  XMStoreFloat4x4(&World, world);
}

void Transform::UpdateWorldViewProj(const XMFLOAT4X4 &View,
                                    const XMFLOAT4X4 &Proj) {
  XMMATRIX world = XMLoadFloat4x4(&World);
  XMMATRIX view = XMLoadFloat4x4(&View);
  XMMATRIX proj = XMLoadFloat4x4(&Proj);
  XMMATRIX wvp = world * view * proj;
  XMStoreFloat4x4(&WorldViewProj, wvp);
}

void Transform::UpdateCartesian() {
  // 球坐标系转直角坐标系公式（标准 3D 数学公式）
  // X = r * sin(phi) * cos(theta)
  // Y = r * cos(phi)
  // Z = r * sin(phi) * sin(theta)
  const float sinPhi = sinf(Phi);
  const float cosPhi = cosf(Phi);
  const float cosTheta = cosf(Theta);
  const float sinTheta = sinf(Theta);

  Position.x = Radius * sinPhi * cosTheta;
  Position.y = Radius * cosPhi;
  Position.z = Radius * sinPhi * sinTheta;
}

void Transform::UpdateSpherical() {
  // 直角坐标转球坐标
  const float x = Position.x;
  const float y = Position.y;
  const float z = Position.z;

  // 半径：到原点距离
  Radius = sqrtf(x * x + y * y + z * z);

  if (Radius < 0.0001f) {
    // 避免除零，原点时角度归零
    Theta = 0.0f;
    Phi = 0.0f;
    return;
  }

  // 极角 Phi：从 Y 轴向下量的角度
  Phi = acosf(y / Radius);

  // 方位角 Theta：水平绕 Y 轴旋转
  Theta = atan2f(z, x);
}

std::ostream &z8::operator<<(std::ostream &o, const Transform &transform) {
  o << "x: " << transform.Position.x << ", y: " << transform.Position.y
    << ", z: " << transform.Position.z;
  return o;
}