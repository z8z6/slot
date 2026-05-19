//
// Created by zhou_zhengming on 2026/5/11.
//

#include "UI/Mesh/RectangleMesh.h"
#include "Util/Color.h"

using namespace DirectX;
using namespace z8;

z8::RectangleMesh::RectangleMesh() {
  V = {Vertex({{-1.0f, -1.0f, 0.0f}, XMFLOAT4(Color::Black_1)}),
       Vertex({{-1.0f, +1.0f, 0.0f}, XMFLOAT4(Color::Black_1)}),
       Vertex({{+1.0f, +1.0f, 0.0f}, XMFLOAT4(Color::Black_1)}),
       Vertex({{+1.0f, -1.0f, 0.0f}, XMFLOAT4(Color::Black_1)})};

  I = {// front face
       {0, 1, 2},
       {0, 2, 3}};
}