//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once
#include <DirectXMath.h>

namespace z8 {
class Math {
public:
  inline static float PI = DirectX::XM_PI;

  static DirectX::XMFLOAT4X4 Identity4x4()
  {
    static DirectX::XMFLOAT4X4 I(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    return I;
  }

  template<typename T>
  static T Clamp(const T& x, const T& low, const T& high)
  {
    return x < low ? low : (x > high ? high : x);
  }


};

}
