//
// Created by zhou_zhengming on 2026/5/13.
//
#pragma once
#include <DirectXMath.h>

namespace z8
{
struct Transform
{
  DirectX::XMFLOAT3 Position;
  DirectX::XMFLOAT3 Rotation;
  DirectX::XMFLOAT3 Scale;

  Transform();

  DirectX::XMMATRIX GetWorldMatrix() const;
};
}
