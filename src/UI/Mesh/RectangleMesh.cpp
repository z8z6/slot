//
// Created by zhou_zhengming on 2026/5/11.
//

#include "UI/Mesh/RectangleMesh.h"
#include <DirectXColors.h>

using namespace DirectX;
using namespace z8;

z8::RectangleMesh::RectangleMesh()
{
  V = {
    Vertex({{-1.0f, -1.0f, 0.0f}, XMFLOAT4(Colors::White)}),
    Vertex({{-1.0f, +1.0f, 0.0f}, XMFLOAT4(Colors::Black)}),
    Vertex({{+1.0f, +1.0f, 0.0f}, XMFLOAT4(Colors::Red)}),
    Vertex({{+1.0f, -1.0f, 0.0f}, XMFLOAT4(Colors::Green)})
  };

  I = {
    // front face
    {0, 1, 2},
    {0, 2, 3}
  };
}