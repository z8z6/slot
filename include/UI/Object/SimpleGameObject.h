//
// Created by zhou_zhengming on 2026/5/20.
//

#pragma once

#include "GameObject.h"

namespace z8 {
class SimpleGameObject : public GameObjectImpl<DirectX::XMFLOAT4X4> {
public:
  void Update(Camera*, Timer*) override;
};
}




