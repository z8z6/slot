//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once
#include <DirectXMath.h>

namespace z8 {
class Math {
public:
  inline static float PI = 3.1415926535f;


  static DirectX::XMFLOAT4X4 Identity4x4()
  {
    static DirectX::XMFLOAT4X4 I(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    return I;
  }

};

}
