//
// Created by zhou_zhengming on 2026/5/20.
//

#pragma once

#include "GameObject.h"

namespace z8 {
class SimpleGameObject : public GameObjectImpl<DirectX::XMFLOAT4X4> {
public:
  void Update(const DirectX::XMFLOAT4X4& View, const DirectX::XMFLOAT4X4& Proj) override;
};
}




