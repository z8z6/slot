//
// Created by zhou_zhengming on 2026/5/21.
//

#pragma once
#include "Light.h"

namespace z8 {
class ParallelLight : public Light{
public:
  DirectX::XMFLOAT3 Direction;
  ParallelLight();
};
}


