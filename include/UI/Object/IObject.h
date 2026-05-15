//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once
#include "Transform.h"
#include "Core/Event.h"

namespace z8
{
class IMesh;
class IMaterial;
class ICollider;

class IObject {
public:
  IMesh* Mesh;
  IMaterial* Material;
  ICollider* Collider;
  Transform Transform;

  IObject();
  virtual ~IObject() = default;
  virtual void* ConstBuf() { return nullptr; }
  virtual unsigned ConstBufSize() { return 0; }
  virtual void Update(const DirectX::XMFLOAT4X4& View, const DirectX::XMFLOAT4X4&Proj) {}
  virtual void OnMouseUp(ButtonEventArgs) {}
  virtual void OnMouseMove(ButtonEventArgs) {}
  virtual void OnMouseDown(ButtonEventArgs) {}
};
}

