//
// Created by zhou_zhengming on 2026/5/19.
//

#include "Mesh/BuiltinMesh.h"
#include <cmath>
#include <limits>

using namespace z8;
using namespace DirectX;

GridMesh::GridMesh() {
  Id = builtin::mesh::GridMesh;
  Update();
}

void GridMesh::Update() {
  const size_t vertexCount = static_cast<size_t>(DivZ) * DivX;
  if (!std::isfinite(LengthX) || !std::isfinite(LengthZ) || LengthX <= 0.0f ||
      LengthZ <= 0.0f || DivZ < 2 || DivX < 2 ||
      vertexCount > static_cast<size_t>(std::numeric_limits<IndexTy>::max()) + 1)
    return;
  const size_t faceCount = static_cast<size_t>(DivZ - 1) * (DivX - 1) * 2;

  float halfWidth = 0.5f * LengthX;
  float halfDepth = 0.5f * LengthZ;

  float dx = LengthX / (DivX - 1);
  float dz = LengthZ / (DivZ - 1);

  float du = 1.0f / (DivX - 1);
  float dv = 1.0f / (DivZ - 1);

  V.resize(vertexCount);
  for (unsigned i = 0; i < DivZ; ++i) {
    float z = halfDepth - i * dz;
    for (unsigned j = 0; j < DivX; ++j) {
      float x = -halfWidth + j * dx;

      V[i * DivX + j] = Vertex({x, 0.0f, z}, {j * du, i * dv});
    }
  }

  I.resize(faceCount * 3);

  // Iterate over each quad and compute indices.
  size_t k = 0;
  for (unsigned i = 0; i < DivZ - 1; ++i) {
    for (unsigned j = 0; j < DivX - 1; ++j) {
      const auto current = static_cast<IndexTy>(i * DivX + j);
      const auto right = static_cast<IndexTy>(i * DivX + j + 1);
      const auto next = static_cast<IndexTy>((i + 1) * DivX + j);
      const auto nextRight = static_cast<IndexTy>((i + 1) * DivX + j + 1);
      // +Y 朝上的平面从相机侧观察为顺时针，符合 DX12 默认正面约定。
      I[k] = current;
      I[k + 1] = right;
      I[k + 2] = next;
      I[k + 3] = next;
      I[k + 4] = right;
      I[k + 5] = nextRight;

      k += 6; // next quad
    }
  }
}

