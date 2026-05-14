//
// Created by zhou_zhengming on 2026/5/14.
//
#pragma once

#include <cassert>
#include "IMaterial.h"
#include "Target/DirectX/DX12Shader.h"

namespace z8
{
struct RotateCubeMaterial : public IMaterial{
  RotateCubeMaterial()
  {
    V = DX12ShaderRegistry::Instance().GetShader("CubeV");
    P = DX12ShaderRegistry::Instance().GetShader("CubeP");

    assert(V && P);
  }
};
}


