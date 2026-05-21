//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once
#include <cassert>

#include "Material.h"
#include "Target/DirectX/DX12Shader.h"

namespace z8
{
struct  DefaultMaterial : public Material {
  DefaultMaterial()
  {
    V = DX12ShaderRegistry::Instance().GetShader("Default_V");
    P = DX12ShaderRegistry::Instance().GetShader("Default_P");

    assert(V && P);
  }
};
}

