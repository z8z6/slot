//
// Created by zhou_zhengming on 2026/5/20.
//

#include "Util/MeshGenerator.h"
#include <DirectXMath.h>

using namespace z8;
using namespace DirectX;

Vertex MeshGenerator::MidPoint(const Vertex &v0, const Vertex &v1) {
  XMVECTOR p0 = XMLoadFloat3(&v0.Pos);
  XMVECTOR p1 = XMLoadFloat3(&v1.Pos);

  XMVECTOR pos = XMVectorAdd(p0,p1);
  pos = XMVectorScale(pos, 0.5f);

  Vertex v{};
  XMStoreFloat3(&v.Pos, pos);

  return v;
}