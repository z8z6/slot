//
// Created by zhou_zhengming on 2026/5/21.
//

#include "Mesh/BuiltinMesh.h"

using namespace z8;
using namespace DirectX;

MountainMesh::MountainMesh()
{
  for (auto& v : V)
    v.Pos.y = GetHeight(v.Pos.x, v.Pos.z);
}

float MountainMesh::GetHeight(float x, float z)const
{
  return 0.3f*(z*sinf(0.1f*x) + x*cosf(0.1f*z));
}
