//
// Created by zhou_zhengming on 2026/5/12.
//

#include "UI/Object/Object.h"
#include "UI/Material/DefaultMaterial.h"
#include "UI/Material/MissingMaterial.h"

z8::Object::Object()
{
  Material = new DefaultMaterial();
}
