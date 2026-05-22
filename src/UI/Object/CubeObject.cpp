//
// Created by zhou_zhengming on 2026/5/12.
//

#include "UI/Object/CubeObject.h"
#include "UI/Mesh/CubeMesh.h"
#include "UI/Mesh/MeshRegistry.h"

using namespace DirectX;
using namespace z8;

z8::CubeObject::CubeObject()
{
  Mesh = MeshRegistry::Instance().Get("Cube");
}
