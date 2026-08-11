//
// Created by zhou_zhengming on 2026/5/13.
//

#pragma once

#include <DirectXMath.h>

namespace z8
{
class Collider {
public:
  Collider() = default;
  virtual ~Collider() = default;

  virtual bool Contains(DirectX::XMFLOAT3) = 0;
};
}






