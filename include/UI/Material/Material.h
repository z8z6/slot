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
  DirectX::XMFLOAT4 Albedo;
  DirectX::XMFLOAT3 FresnelR0;

  Material() = default;
  virtual ~Material() = default;
};
}






