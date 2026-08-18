//
// Created by zhou_zhengming on 2026/5/20.
//

#include "Object/GameObject/SphereObject.h"
#include "Resource/BuiltinResource.h"

z8::SphereObject::SphereObject() {
  Renderable.Mesh = ResourceRef<BaseMesh>(builtin::mesh::SphereMesh);
}
