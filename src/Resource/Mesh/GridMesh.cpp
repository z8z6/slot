//
// Created by zhou_zhengming on 2026/5/19.
//

#include "Mesh/BuiltinMesh.h"
#include <cmath>
#include <limits>

using namespace z8;
using namespace DirectX;

z8::GridMesh::GridMesh() {
  Id = builtin::mesh::GridMesh;
  float width = 100;
  float depth = 100;
  unsigned m = 101;
  unsigned n = 101;
  const size_t vertexCount = static_cast<size_t>(m) * n;
  if (!std::isfinite(width) || !std::isfinite(depth) || width <= 0.0f ||
      depth <= 0.0f || m < 2 || n < 2 ||
      vertexCount > static_cast<size_t>(std::numeric_limits<IndexTy>::max()) + 1)
    return;
  const size_t faceCount = static_cast<size_t>(m - 1) * (n - 1) * 2;

  float halfWidth = 0.5f * width;
  float halfDepth = 0.5f * depth;

  float dx = width / (n - 1);
  float dz = depth / (m - 1);

  float du = 1.0f / (n - 1);
  float dv = 1.0f / (m - 1);

  V.resize(vertexCount);
  for (unsigned i = 0; i < m; ++i) {
    float z = halfDepth - i * dz;
    for (unsigned j = 0; j < n; ++j) {
      float x = -halfWidth + j * dx;

      V[i * n + j] = Vertex({x, 0.0f, z}, {j * du, i * dv});
    }
  }

  I.resize(faceCount * 3);

  // Iterate over each quad and compute indices.
  size_t k = 0;
  for (unsigned i = 0; i < m - 1; ++i) {
    for (unsigned j = 0; j < n - 1; ++j) {
      const auto current = static_cast<IndexTy>(i * n + j);
      const auto right = static_cast<IndexTy>(i * n + j + 1);
      const auto next = static_cast<IndexTy>((i + 1) * n + j);
      const auto nextRight = static_cast<IndexTy>((i + 1) * n + j + 1);
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

