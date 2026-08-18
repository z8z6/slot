//
// Created by zhou_zhengming on 2026/5/21.
//

#include "Mesh/BuiltinMesh.h"

using namespace z8;
using namespace DirectX;

namespace {
float GetHeight(float x, float z)
{
  return 0.3f*(z*sinf(0.1f*x) + x*cosf(0.1f*z));
}
}

MountainMesh::MountainMesh()
{
  Id = builtin::mesh::MountainMesh;
  Func = GetHeight;
  MountainMesh::Update();
}

void MountainMesh::Update() {
  GridMesh::Update();
  for (auto& v : V)
    v.Pos.y = Func(v.Pos.x, v.Pos.z);
}
