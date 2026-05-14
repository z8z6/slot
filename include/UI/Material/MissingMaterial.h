#pragma once
#include <cassert>

#include "IMaterial.h"
#include "Target/DirectX/DX12Shader.h"

namespace z8
{
struct MissingMaterial : public IMaterial {
  MissingMaterial()
  {
    V = DX12ShaderRegistry::Instance().GetShader("MissingV");
    P = DX12ShaderRegistry::Instance().GetShader("MissingP");

    assert(V && P);
  }
};
}