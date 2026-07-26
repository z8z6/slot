//
// Created by zhou_zhengming on 2026/7/26.
//

#pragma once

#include "Target/DirectX/DX12Common.h"

namespace z8 {
class DX12Buffer : public DX12Common {
public:
  explicit DX12Buffer(DX12Render *R) : DX12Common(R) {}
  virtual void Init(unsigned size) = 0;
  virtual void Update(const void *src) = 0;
};

}


