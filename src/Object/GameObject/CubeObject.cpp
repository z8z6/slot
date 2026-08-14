//
// Created by zhou_zhengming on 2026/5/12.
//

#include "Object/GameObject/CubeObject.h"
#include "Resource/BuiltinResource.h"

using namespace DirectX;
using namespace z8;

z8::CubeObject::CubeObject()
{
  Renderable.Mesh = ResourceRef<Mesh>(builtin::CubeMesh);
}
