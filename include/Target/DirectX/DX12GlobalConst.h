//
// Created by zhou_zhengming on 2026/5/22.
//

#pragma once
#include <DirectXMath.h>

namespace z8 {
class DX12Render;
// 必须注意对齐，GPU侧的类布局可能不一致
struct  DX12GlobalConst {
  DirectX::XMFLOAT4X4A ViewProj;
  float TimeCost;
  float TimeTotal;

  inline static unsigned Index = 0;
  void Update(DX12Render* R);
};
}




