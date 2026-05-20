//
// Created by zhou_zhengming on 2026/5/20.
//

#include "UI/Object/SkullObject.h"
#include "UI/Mesh/MeshRegistry.h"

using namespace z8;

z8::SkullObject::SkullObject() {
  Mesh = MeshRegistry::Instance().GetMesh("Skull");
}