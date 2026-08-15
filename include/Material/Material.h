//
// Created by zhou_zhengming on 2026/5/12.
//
#pragma once
#include <DirectXColors.h>
#include <DirectXMath.h>
#include "Resource/ResourceHandle.h"
#include <string>

namespace z8
{
struct ShaderProgram;
class Texture;
class Material {
public:
  DirectX::XMFLOAT4 Albedo = DirectX::XMFLOAT4(DirectX::Colors::ForestGreen);
  /** 可选基础色纹理；为空时 Shader 只使用 Albedo。 */
  ResourceRef<Texture> BaseColorTexture;
  DirectX::XMFLOAT3 FresnelR0 = { 0.02f, 0.02f, 0.02f };
  float Rough = 0.25f;
  std::string Name;
  // Material 决定绘制算法；场景对象不应直接了解 VS/PS 或 PSO。
  ResourceRef<ShaderProgram> Program;

  Material() = default;
  virtual ~Material() = default;

  /** 返回资源注册使用的稳定名称；普通运行时材质应在导入边界显式提供 ID。 */
  virtual std::string GetName() const { return {}; }
};
}






