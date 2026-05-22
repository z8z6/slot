//
// Created by zhou_zhengming on 2026/5/20.
//

#include "UI/Object/SphereObject.h"
#include "UI/Mesh/MeshRegistry.h"

z8::SphereObject::SphereObject() {
  Mesh = MeshRegistry::Instance().Get("Sphere");
}