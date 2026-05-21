//
// Created by zhou_zhengming on 2026/5/11.
//

#include "UI/Mesh/RectangleMesh.h"
#include "Util/Color.h"
#include "UI/Mesh/MeshRegistry.h"

using namespace DirectX;
using namespace z8;

static MeshRegister<RectangleMesh> R;

z8::RectangleMesh::RectangleMesh() {
  V = {
    Vertex(-1.0f, -1.0f, 0.0f),
       Vertex(-1.0f, +1.0f, 0.0f),
       Vertex(+1.0f, +1.0f, 0.0f),
       Vertex(+1.0f, -1.0f, 0.0f)
  };

  I = {// front face
       0, 1, 2,
       0, 2, 3};

  Name = "Rect";
}