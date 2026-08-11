//
// Created by zhou_zhengming on 2026/5/11.
//

#include "Mesh/CubeMesh.h"

#include <DirectXColors.h>

using namespace z8;
using namespace DirectX;

CubeMesh::CubeMesh() {
  V = {Vertex(-1.0f, -1.0f, -1.0f),
       Vertex(-1.0f, +1.0f, -1.0f),
       Vertex(+1.0f, +1.0f, -1.0f),
       Vertex(+1.0f, -1.0f, -1.0f),
       Vertex(-1.0f, -1.0f, +1.0f),
       Vertex(-1.0f, +1.0f, +1.0f),
       Vertex(+1.0f, +1.0f, +1.0f),
       Vertex(+1.0f, -1.0f, +1.0f)
  };

  I = {// front face
       0, 1, 2,
       0, 2, 3,

       // back face
       4, 6, 5,
       4, 7, 6,

       // left face
       4, 5, 1,
       4, 1, 0,

       // right face
       3, 2, 6,
       3, 6, 7,

       // top face
       1, 5, 6,
       1, 6, 2,

       // bottom face
       4, 0, 3,
       4, 3, 7};

  Name = "Cube";
}
