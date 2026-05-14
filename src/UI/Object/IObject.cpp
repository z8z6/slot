//
// Created by zhou_zhengming on 2026/5/12.
//

#include "UI/Object/IObject.h"
#include "UI/Material/DefaultMaterial.h"

z8::IObject::IObject()
{
  Material = new DefaultMaterial;
}
