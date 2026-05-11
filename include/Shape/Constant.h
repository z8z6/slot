//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once
#include <DirectXMath.h>

namespace z8 {
class Constant {
public:
  DirectX::XMFLOAT4X4 WorldViewProj {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
  };
};
}

