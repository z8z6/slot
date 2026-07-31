//
// Created by zhou_zhengming on 2026/5/11.
//

#include "UI/Mesh/MeshRegistry.h"
#include "UI/Mesh/RectMesh.h"
#include "Util/Color.h"

using namespace DirectX;
using namespace z8;

static MeshRegister<RectMesh> R;

/**
 * 1 -- 2
 * |    |
 * 0 -- 3
 */
z8::RectMesh::RectMesh() {
  V = {
    Vertex(-0.5f, -0.5f, 0.0f),
       Vertex(-0.5f, 0.5f, 0.0f),
       Vertex(0.5f, 0.5f, 0.0f),
       Vertex(0.5f, -0.5f, 0.0f)
  };

  I = {
    // 正面
    0, 1, 2,
    0, 2, 3,
    // 反面
    0, 2, 1,
    0, 3, 2
  };

  Name = "Rect";
}