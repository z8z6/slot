//
// Created by zhou_zhengming on 2026/5/17.
//

#pragma once

#include "../Object.h"

namespace z8
{
class Mesh;
class Material;
class Collider;
class Shader;
/**
 * @brief 有形状的渲染物体，一般包括：
 * 1. 网格
 * 2. 材质
 * 3. shader
 * 4. 常量缓冲区
 *
 * 默认情况下，每个 GameObject 都会包含一个世界矩阵的常量
 */
class GameObject : public Object{
public:
  Mesh* Mesh;
  Material* Material;
  Collider* Collider;
  Shader* PixelShader;
  Shader* VertexShader;

  GameObject();

  virtual void* ConstBuf() = 0;
  virtual unsigned ConstBufSize() = 0;
};

/**
 * @brief 包含常量缓冲区的物体
 * @tparam T 常量缓冲区的类型
 */
template <typename T>
class GameObjectImpl : public GameObject{
protected:
  T Const;
public:
  void* ConstBuf() override { return &Const; }
  unsigned ConstBufSize() override { return sizeof(T); }
};
}






