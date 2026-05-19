#pragma once
#include <cassert>

#include "Material.h"
#include "Target/DirectX/DX12Shader.h"

namespace z8 {
struct MissingMaterial : public Material {
  MissingMaterial() {
    V = DX12ShaderRegistry::Instance().GetShader("MissingV");
    P = DX12ShaderRegistry::Instance().GetShader("MissingP");

    assert(V && P);
  }
};
}