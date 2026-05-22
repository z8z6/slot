//
// Created by zhou_zhengming on 2026/5/22.
//

#pragma once
#include <DirectXMath.h>

namespace z8 {
class DX12Render;
struct  DX12GlobalConst {
  float TimeCost;
  float TimeTotal;
  DirectX::XMFLOAT4X4 ViewProj;

  inline static unsigned Index = 0;
  void Update(DX12Render* R);
};
}




