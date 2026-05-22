//
// Created by zhou_zhengming on 2026/5/12.
//
#pragma once
#include <DirectXMath.h>
#include <string>

namespace z8
{
class Material {
public:
  std::string Name;
  DirectX::XMFLOAT4 Albedo = { 1.0f, 1.0f, 1.0f, 1.0f };
  DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f };
  float Rough = 0.25f;

  Material() = default;
  virtual ~Material() = default;
};
}






