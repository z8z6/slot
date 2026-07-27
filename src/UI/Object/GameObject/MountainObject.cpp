//
// Created by zhou_zhengming on 2026/5/21.
//

#include "UI/Object/GameObject/MountainObject.h"

#include "UI/Mesh/MeshRegistry.h"

using namespace z8;

z8::MountainObject::MountainObject() {
  Mesh = MeshRegistry::Instance().Get("Mountain");
}