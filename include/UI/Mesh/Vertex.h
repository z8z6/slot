//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once

#include <DirectXMath.h>

namespace z8 {
class Vertex {
public:
  DirectX::XMFLOAT3 Pos;
  DirectX::XMFLOAT3 Normal;

  Vertex() = default;
  Vertex(DirectX::XMFLOAT3 P) : Pos(P), Normal() {}
  Vertex(float x, float y, float z) : Pos(x,y,z), Normal() {}
};
}
