//
// Created by zhou_zhengming on 2026/5/20.
//

#include "Object/GameObject/SkullObject.h"
#include "Resource/BuiltinResource.h"

using namespace z8;

z8::SkullObject::SkullObject() {
  Renderable.Mesh = ResourceRef<Mesh>(builtin::SkullMesh);
}
