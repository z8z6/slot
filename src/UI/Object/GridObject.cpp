//
// Created by zhou_zhengming on 2026/5/19.
//

#include "UI/Object/GridObject.h"
#include "UI/Mesh/MeshRegistry.h"

using namespace z8;
using namespace DirectX;

GridObject::GridObject() {
  Mesh = MeshRegistry::Instance().Get("Grid");
}

