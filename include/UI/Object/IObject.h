//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once
#include <DirectXMath.h>

namespace z8
{
class IMesh;
class IMaterial;

class IObject {
public:
  IMesh* Mesh;
  IMaterial* Material;
  DirectX::XMFLOAT4X4 WorldViewProj;

  IObject() = default;
  virtual ~IObject() = default;
};
}

