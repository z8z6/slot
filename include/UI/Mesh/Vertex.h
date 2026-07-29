//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once

#include <DirectXMath.h>

namespace z8 {
/**
 * @brief 对应顶点着色器的输入结构体
 * 1. 相对坐标
 * 2. 顶点法线
 * 3. 顶点纹理坐标
 */
class Vertex {
public:
  DirectX::XMFLOAT3 Pos;
  DirectX::XMFLOAT3 Normal;
  DirectX::XMFLOAT2 TexCoord;

  Vertex() = default;
  Vertex(DirectX::XMFLOAT3 P) : Pos(P), Normal(), TexCoord() {}
  Vertex(float x, float y, float z) : Pos(x,y,z), Normal(), TexCoord() {}
};
}
