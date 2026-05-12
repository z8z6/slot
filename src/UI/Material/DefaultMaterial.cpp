//
// Created by zhou_zhengming on 2026/5/12.
//

#include "UI/Material/DefaultMaterial.h"
#include "Target/DirectX/DX12Shader.h"

using namespace z8;

z8::DefaultMaterial::DefaultMaterial()
{
  V = DX12ShaderRegistry::Instance().GetShader("DV");
  P = DX12ShaderRegistry::Instance().GetShader("DP");

  assert(V && P);
}