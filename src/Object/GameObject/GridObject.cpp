//
// Created by zhou_zhengming on 2026/5/19.
//

#include "Object/GameObject/GridObject.h"
#include "Resource/BuiltinResource.h"

using namespace z8;
using namespace DirectX;

GridObject::GridObject() {
  Renderable.Mesh = ResourceReference<Mesh>(builtin::GridMesh);
}

