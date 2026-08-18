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
struct BaseShaderProgram;
class BaseTexture;
/**
 * @brief 材质
 * 基于 Cook‑Torrance BRDF
 */
struct BaseMaterial : Resource{
  DirectX::XMFLOAT4 Albedo{};       // 反照率，漫反射基础色
  DirectX::XMFLOAT3 FresnelR0{};    // 菲涅尔反射率
  float Rough;                      // 粗糙度，决定镜面高光
  ResourceRef<BaseTexture> Texture;
  ResourceRef<BaseShaderProgram> Program;

  BaseMaterial() {
    Albedo = DirectX::XMFLOAT4(DirectX::Colors::ForestGreen);
    FresnelR0 = { 0.02f, 0.02f, 0.02f };
    Rough = 0.25f;
    Type = ResourceTy::Material;
    Id = builtin::material::MaterialPrefix;
    Program = ResourceRef<BaseShaderProgram>(builtin::shader::program::GameObjectProgram);
  }
};
}






