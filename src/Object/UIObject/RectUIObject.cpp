//
// Created by zhou_zhengming on 2026/7/27.
//

#include "Object/UIObject/RectUIObject.h"
#include "Resource/BuiltinResource.h"

using namespace z8;

RectUIObject::RectUIObject() {
  Renderable.Mesh = ResourceReference<Mesh>(builtin::RectMesh);
}
