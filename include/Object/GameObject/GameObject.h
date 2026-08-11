//
// Created by zhou_zhengming on 2026/5/17.
//

#pragma once

#include "Object/Object.h"
#include "Resource/ResourceHandle.h"

namespace z8
{
class Material;
class Mesh;
class Collider;

/**
 * @brief 场景对象可渲染部分的持久化资源绑定。
 *
 * Reference 允许场景在资源尚未驻留时存在；渲染器构建 RenderItem 时统一解析成
 * Handle，之后的帧循环不再进行字符串查询。
 */
struct RenderableComponent {
  ResourceReference<Mesh> Mesh;
  ResourceReference<Material> Material;
};

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
  // Renderable 保存可序列化的资源引用，实际资源所有权统一位于 ResourceManager。
  RenderableComponent Renderable;
  Collider* Collider;

  GameObject();
  ~GameObject() override;

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






