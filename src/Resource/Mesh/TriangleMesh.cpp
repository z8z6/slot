//
// Created by zhou_zhengming on 2026/5/11.
//

#include "Mesh/BuiltinMesh.h"

using namespace DirectX;
using namespace z8;

TriangleMesh::TriangleMesh() {
  Id = builtin::mesh::TriangleMesh;
  // 双面三角形使用两组顶点，确保自动法线不会在同一位置互相抵消。
  V = {Vertex({-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}),
       Vertex({0.0f, 0.5f, 0.0f}, {0.5f, 0.0f}),
       Vertex({0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}),
       Vertex({-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f}),
       Vertex({0.5f, -0.5f, 0.0f}, {0.0f, 1.0f}),
       Vertex({0.0f, 0.5f, 0.0f}, {0.5f, 0.0f})};
  I = {0, 1, 2, 3, 4, 5};
}
