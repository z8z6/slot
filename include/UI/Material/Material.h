//
// Created by zhou_zhengming on 2026/5/12.
//
#pragma once
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <string>

namespace z8
{
class Material {
public:
  DirectX::XMFLOAT4 Albedo = DirectX::XMFLOAT4(DirectX::Colors::ForestGreen);
  DirectX::XMFLOAT3 FresnelR0 = { 0.02f, 0.02f, 0.02f };
  float Rough = 0.25f;
  std::string Name;

  Material() = default;
  virtual ~Material() = default;
};
}






