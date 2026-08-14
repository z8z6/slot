//
// Created by zhou_zhengming on 2026/5/11.
//

#include "Mesh/BuiltinMesh.h"

using namespace DirectX;
using namespace z8;

RectMesh::RectMesh() {
  // 正反面不能共享顶点，否则面积加权时方向相反的法线会彼此抵消。
  V = {
      Vertex({-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}),
      Vertex({-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f}),
      Vertex({0.5f, 0.5f, 0.0f}, {1.0f, 0.0f}),
      Vertex({0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}),
      Vertex({-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}),
      Vertex({0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}),
      Vertex({0.5f, 0.5f, 0.0f}, {0.0f, 0.0f}),
      Vertex({-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f}),
  };
  I = {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7};
  Name = "Rect";
}
