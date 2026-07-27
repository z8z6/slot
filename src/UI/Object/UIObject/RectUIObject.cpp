//
// Created by zhou_zhengming on 2026/7/27.
//

#include "UI/Object/UIObject/RectUIObject.h"
#include "UI/Mesh/MeshRegistry.h"

using namespace z8;

RectUIObject::RectUIObject() {
  Mesh = MeshRegistry::Instance().Get("Rect");
}