//
// Created by zhou_zhengming on 2026/5/17.
//

#include "Object/GameObject/GameObject.h"
#include "Resource/BuiltinResource.h"

#include <cmath>

using namespace DirectX;

z8::GameObject::GameObject()
    : Collider(nullptr) {
  // 普通场景对象共享默认材质和 Program；派生对象只需要选择几何资源。
  Renderable.Material =
      ResourceRef<BaseMaterial>(builtin::material::GrassBlockMaterial);
}

z8::GameObject::~GameObject() = default;

void z8::ObjectTransformConst::Update(const XMFLOAT4X4& world) {
  const XMMATRIX worldMatrix = XMLoadFloat4x4(&world);
  XMStoreFloat4x4(&World, XMMatrixTranspose(worldMatrix));

  XMVECTOR determinant;
  XMMATRIX inverse = XMMatrixInverse(&determinant, worldMatrix);
  if (std::abs(XMVectorGetX(determinant)) <= 1.0e-8f)
    inverse = XMMatrixIdentity();
  // 行向量法线使用 inverse-transpose；HLSL 列主序上传还需再转置，因此内存中
  // 恰好写入 inverse。奇异缩放没有可定义法线，使用单位矩阵保证输出保持有限。
  XMStoreFloat4x4(&WorldInvTranspose, inverse);
}
