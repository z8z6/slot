//
// Created by zhou_zhengming on 2026/5/15.
//

#pragma once
#include "UIObject.h"

namespace z8
{
class RectObject : public UIObject
{
public:
  void* ConstBuf() override;
  unsigned ConstBufSize() override;
  void Update(const DirectX::XMFLOAT4X4& View, const DirectX::XMFLOAT4X4& Proj) override;

private:
  DirectX::XMFLOAT4X4 objConstants;
public:
  RectObject();
};
}
