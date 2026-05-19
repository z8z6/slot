//
// Created by zhou_zhengming on 2026/5/19.
//

#include "UI/Mesh/GridMesh.h"
#include <DirectXColors.h>

using namespace z8;
using namespace DirectX;

z8::GridMesh::GridMesh(float width, float depth, unsigned m, unsigned n) {
  unsigned vertexCount = m * n;
  unsigned faceCount = (m - 1) * (n - 1) * 2;

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

      V[i * n + j] = {XMFLOAT3(x, 0.1f, z), XMFLOAT4(Colors::White)};
    }
  }

  I.resize(faceCount * 3);

  // Iterate over each quad and compute indices.
  unsigned k = 0;
  for (unsigned i = 0; i < m - 1; ++i) {
    for (unsigned j = 0; j < n - 1; ++j) {
      I[k]   = i * n + j;
      I[k+1] = (i+1) * n + j;
      I[k+2] = i * n + j + 1;

      I[k+3] = (i+1) * n + j;
      I[k+4] = (i+1) * n + j + 1;
      I[k+5] = i * n + j + 1;

      k += 6; // next quad
    }
  }
}

