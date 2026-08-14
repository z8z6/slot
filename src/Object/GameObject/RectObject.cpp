//
// Created by zhou_zhengming on 2026/5/15.
//
#include "Object/GameObject/RectObject.h"

#include "Resource/BuiltinResource.h"

using namespace z8;
using namespace DirectX;

RectObject::RectObject()
{
  Renderable.Mesh = ResourceRef<Mesh>(builtin::RectMesh);
}
