//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once

#include "Material.h"
#include "Resource/BuiltinResource.h"

namespace z8
{
struct MetalMaterial : public Material {
  MetalMaterial(){
    Name = "Metal";
    Program = ResourceReference<ShaderProgram>(builtin::GameObjectProgram);
  }
};
}

