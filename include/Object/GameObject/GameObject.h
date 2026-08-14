//
// Created by zhou_zhengming on 2026/5/17.
//

#pragma once

#include "Object/Object.h"
#include "Resource/ResourceHandle.h"

#include <string>

namespace z8
{
class Material;
class Mesh;
class Collider;

/**
 * @brief 普通和 UI 顶点着色器共享的对象变换 ABI。
 *
 * World 把位置变换到世界空间；WorldInvTranspose 抵消非均匀缩放，使法线仍与
 * 变换后的曲面切线正交。两个矩阵均按 HLSL 列主序常量的存储约定写入。
 */
struct ObjectTransformConst {
  DirectX::XMFLOAT4X4 World;
  DirectX::XMFLOAT4X4 WorldInvTranspose;

  /** 从 DirectXMath 行主序世界矩阵同步生成 Shader 所需的两个矩阵。 */
  void Update(const DirectX::XMFLOAT4X4& world);
};
static_assert(sizeof(ObjectTransformConst) == 128,
              "ObjectTransformConst must match the first two b0 matrices.");

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
  /** 编辑器显示名称属于场景数据，不能只保存在 TreeView 或 Details 控件中。 */
  std::string Name;
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






