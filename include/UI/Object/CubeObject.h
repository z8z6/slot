//
// Created by zhou_zhengming on 2026/5/12.
//
#pragma once
#include "GameObject.h"

namespace z8
{
class CubeObject : public GameObject{
public:
  void Update(const DirectX::XMFLOAT4X4& View, const DirectX::XMFLOAT4X4& Proj) override;
  void* ConstBuf() override;
  unsigned ConstBufSize() override;

private:
  DirectX::XMFLOAT4X4 objConstants;
public:

  CubeObject();
};
}


