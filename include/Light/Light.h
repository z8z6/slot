//
// Created by zhou_zhengming on 2026/5/21.
//

#pragma once
#include "Object/Object.h"

namespace z8 {
class Light : public Object {
public:
  DirectX::XMFLOAT3 Color = {1,1,1};
  DirectX::XMFLOAT3 Direction = {1,0,1};
};
}




