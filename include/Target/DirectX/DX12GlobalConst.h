//
// Created by zhou_zhengming on 2026/5/22.
//

#pragma once
#include <DirectXMath.h>

namespace z8 {
class DX12Render;

struct DX12Light {
  DirectX::XMFLOAT3 Position;
  float p0;
  DirectX::XMFLOAT3 Strength;
  float p1;
  DirectX::XMFLOAT3 Direction;
  float p2;
};

// 必须注意对齐，GPU侧的类布局可能不一致
struct  DX12GlobalConst {
  DirectX::XMFLOAT4X4A ViewProj;
  DX12Light Light;
  DirectX::XMFLOAT4 AmbientLight;
  DirectX::XMFLOAT3 Camera;
  float TimeCost;
  float TimeTotal;

  inline static unsigned Index = 0;
  void Update(DX12Render* R);
};


}




