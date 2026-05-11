//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once

#include <DirectXMath.h>

namespace z8 {
class Vertex {
public:
  DirectX::XMFLOAT3 Pos;
  DirectX::XMFLOAT4 Color;

  Vertex() = default;
};

class VertexGroup {
public:
  uint16_t a;
  uint16_t b;
  uint16_t c;

  VertexGroup() = default;
  constexpr VertexGroup(uint16_t _a, uint16_t _b, uint16_t _c) noexcept
  : a(_a), b(_b), c(_c) {}
};
}
