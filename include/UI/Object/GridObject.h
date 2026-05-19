//
// Created by zhou_zhengming on 2026/5/19.
//

#pragma once
#include "GameObject.h"

namespace z8 {
class GridObject : public GameObject {
public:
  void Update(const DirectX::XMFLOAT4X4 &View,
              const DirectX::XMFLOAT4X4 &Proj) override;

private:
  DirectX::XMFLOAT4X4 objConstants;
public:
  GridObject();
  void *ConstBuf() override;
  unsigned ConstBufSize() override;
};

}




