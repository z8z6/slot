//
// Created by zhou_zhengming on 2026/5/12.
//
#pragma once

namespace z8
{
class DX12Shader;

class Material {
public:
  DX12Shader* V;
  DX12Shader* P;

  Material() = default;
  virtual ~Material() = default;
};
}






