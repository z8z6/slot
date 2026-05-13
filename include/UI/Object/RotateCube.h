//
// Created by zhou_zhengming on 2026/5/13.
//

#pragma once
#include "CubeObject.h"

namespace z8
{
class RotateCube : public CubeObject{
  POINT LastPos;
  float Theta = 1.5f * DirectX::XM_PI;
  float Phi = DirectX::XM_PIDIV4;
  float Radius = 5.0f;
  DirectX::XMFLOAT4X4 objConstants;
public:
  RotateCube();
  void Update(DirectX::XMFLOAT4X4) override;
  void* ConstBuf() override;
  unsigned ConstBufSize() override;
  void OnMouseMove(ButtonEventArgs) override;
};
}
