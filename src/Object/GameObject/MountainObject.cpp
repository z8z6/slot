//
// Created by zhou_zhengming on 2026/5/21.
//

#include "Object/GameObject/MountainObject.h"

#include "Resource/BuiltinResource.h"

using namespace z8;

z8::MountainObject::MountainObject() {
  Renderable.Mesh = ResourceRef<Mesh>(builtin::mesh::MountainMesh);
}
