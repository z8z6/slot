//
// Created by zhou_zhengming on 2026/5/20.
//

#include "UI/Object/AmiyaObject.h"
#include "UI/Mesh/MeshRegistry.h"

z8::AmiyaObject::AmiyaObject() {
  Mesh = MeshRegistry::Instance().GetMesh("Amiya");
}