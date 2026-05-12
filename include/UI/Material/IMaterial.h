//
// Created by zhou_zhengming on 2026/5/12.
//
#pragma once

namespace z8
{
class DX12Shader;

class IMaterial {
public:
  DX12Shader* V;
  DX12Shader* P;

  IMaterial() = default;
  virtual ~IMaterial() = default;
};
}






