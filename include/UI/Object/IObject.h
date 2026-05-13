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
  DirectX::XMMATRIX WorldViewProj;

  IObject() = default;
  virtual ~IObject() = default;
  virtual void* ConstBuf() = 0;
  virtual unsigned ConstBufSize() = 0;
  virtual void Update(DirectX::XMFLOAT4X4) {}
  virtual void OnMouseUp(ButtonEventArgs) {}
  virtual void OnMouseMove(ButtonEventArgs) {}
  virtual void OnMouseDown(ButtonEventArgs) {}
};
}

