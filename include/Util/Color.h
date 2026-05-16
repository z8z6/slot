//
// Created by zhou_zhengming on 2026/5/16.
//

#pragma once

#include <DirectXColors.h>

namespace z8
{
class Color
{
public:
  inline static DirectX::XMVECTORF32 Black_1 = { { { 0.1176f, 0.1216f, 0.1333f, 1.f } } };
  inline static DirectX::XMVECTORF32 Black_2 = { { { 0.1686f, 0.1765f, 0.1882f, 1.f } } };
};
}