//
// Created by zhou_zhengming on 2026/5/15.
//
#include "UI/Object/RectObject.h"

#include "UI/Mesh/MeshRegistry.h"
#include "UI/Mesh/RectangleMesh.h"

using namespace z8;
using namespace DirectX;

RectObject::RectObject()
{
  Mesh = MeshRegistry::Instance().Get("Rect");
}
