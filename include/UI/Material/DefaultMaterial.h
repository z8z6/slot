//
// Created by zhou_zhengming on 2026/5/12.
//

#pragma once
#include <cassert>

#include "IMaterial.h"
#include "Target/DirectX/DX12Shader.h"

namespace z8
{
struct  DefaultMaterial : public IMaterial {
  DefaultMaterial()
  {
    V = DX12ShaderRegistry::Instance().GetShader("DV");
    P = DX12ShaderRegistry::Instance().GetShader("DP");

    assert(V && P);
  }
};
}

