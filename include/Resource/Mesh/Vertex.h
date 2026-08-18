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

  /** 默认清零所有属性，避免导入器只写位置时把未初始化数据上传到 GPU。 */
  Vertex() : Pos{}, Normal{}, TexCoord{} {}
  Vertex(DirectX::XMFLOAT3 position)
      : Pos(position), Normal{}, TexCoord{} {}
  Vertex(DirectX::XMFLOAT3 position, DirectX::XMFLOAT2 texCoord)
      : Pos(position), Normal{}, TexCoord(texCoord) {}
  Vertex(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 normal,
         DirectX::XMFLOAT2 texCoord)
      : Pos(position), Normal(normal), TexCoord(texCoord) {}
  Vertex(float x, float y, float z) : Pos(x, y, z), Normal{}, TexCoord{} {}
};
} // namespace z8
