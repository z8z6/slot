//
// Created by zhou_zhengming on 2026/5/21.
//

#pragma once
#include "Material.h"
#include "Target/DirectX/DX12Shader.h"

#include <cassert>

namespace z8 {
class TimeMaterial : public Material{
public:
  TimeMaterial() {
    V = DX12ShaderRegistry::Instance().GetShader("Time_V");
    P = DX12ShaderRegistry::Instance().GetShader("Time_P");

    assert(V && P);
  }
};
}





