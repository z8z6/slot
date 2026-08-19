//
// Created by zhou_zhengming on 2026/5/13.
//

#pragma once

#include "ResourceRef.h"
#include <DirectXMath.h>

namespace z8
{
struct BaseCollider : Resource{
  BaseCollider() = default;
  virtual bool Contains(DirectX::XMFLOAT3) = 0;
};
}






