//
// Created by zhou_zhengming on 2026/5/13.
//

#pragma once
#include "CubeObject.h"

namespace z8
{
class RotateCube : public CubeObject
{
  POINT LastPos;
  DirectX::XMFLOAT4X4 objConstants;

public:
  RotateCube();
  void Update(const DirectX::XMFLOAT4X4&) override;
  void* ConstBuf() override;
  unsigned ConstBufSize() override;
  void OnMouseMove(ButtonEventArgs) override;
};
}
