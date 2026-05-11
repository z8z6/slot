//
// Created by zhou_zhengming on 2026/5/11.
//

#include "Shape/Cube.h"

#include <DirectXColors.h>

using namespace z8;
using namespace DirectX;

z8::Cube::Cube() {
  Vs = {Vertex({{-1.0f, -1.0f, -1.0f}, XMFLOAT4(Colors::White)}),
        Vertex({{-1.0f, +1.0f, -1.0f}, XMFLOAT4(Colors::Black)}),
        Vertex({{+1.0f, +1.0f, -1.0f}, XMFLOAT4(Colors::Red)}),
        Vertex({{+1.0f, -1.0f, -1.0f}, XMFLOAT4(Colors::Green)}),
        Vertex({{-1.0f, -1.0f, +1.0f}, XMFLOAT4(Colors::Blue)}),
        Vertex({{-1.0f, +1.0f, +1.0f}, XMFLOAT4(Colors::Yellow)}),
        Vertex({{+1.0f, +1.0f, +1.0f}, XMFLOAT4(Colors::Cyan)}),
        Vertex({{+1.0f, -1.0f, +1.0f}, XMFLOAT4(Colors::Magenta)})};

  Is = {// front face
        {0, 1, 2},
        {0, 2, 3},

        // back face
        {4, 6, 5},
        {4, 7, 6},

        // left face
        {4, 5, 1},
        {4, 1, 0},

        // right face
        {3, 2, 6},
        {3, 6, 7},

        // top face
        {1, 5, 6},
        {1, 6, 2},

        // bottom face
        {4, 0, 3},
        {4, 3, 7}};

  Const = Constant();
}