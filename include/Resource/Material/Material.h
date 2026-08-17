//
// Created by zhou_zhengming on 2026/5/12.
//
#pragma once
#include "Resource/BuiltinResource.h"

#include "Resource/ResourceHandle.h"
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <string>

namespace z8
{
struct ShaderProgram;
class Texture;
class Material : public Resource{
public:
  DirectX::XMFLOAT4 Albedo = DirectX::XMFLOAT4(DirectX::Colors::ForestGreen);
  /** 可选基础色纹理；为空时 Shader 只使用 Albedo。 */
  ResourceRef<Texture> BaseColorTexture;
  DirectX::XMFLOAT3 FresnelR0 = { 0.02f, 0.02f, 0.02f };
  float Rough = 0.25f;
  // Material 决定绘制算法；场景对象不应直接了解 VS/PS 或 PSO。
  ResourceRef<ShaderProgram> Program;

  Material() {
    Type = ResourceTy::Material;
    Id = builtin::material::MaterialPrefix;
  }
  virtual ~Material() = default;
};
}






