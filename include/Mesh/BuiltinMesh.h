//
// Created by zhou_zhengming on 2026/8/15.
//

#pragma once
#include "Mesh.h"

#include "Resource/BuiltinResource.h"

#define Def_Mesh(Name, Id)                                                     \
  struct Name##Mesh : Mesh {                                                   \
    Name##Mesh();                                                             \
    /** 返回稳定的小写资源名，使注册处不再重复维护同一 ID。 */                  \
    std::string GetName() const override { return std::string(Id); }           \
};

namespace z8 {
Def_Mesh(Cube, builtin::CubeMesh)
Def_Mesh(Grid, builtin::GridMesh)
Def_Mesh(Rect, builtin::RectMesh)
Def_Mesh(Skull, builtin::SkullMesh)
// Triangle 尚未作为默认场景资源注册，但仍使用同一规范名称规则。
Def_Mesh(Triangle, "builtin://mesh/triangle")

class MountainMesh : public GridMesh {
public:
  MountainMesh();
  float GetHeight(float x, float z) const;
  /** 山地与 Grid 共享拓扑构造，但必须拥有独立资源身份。 */
  std::string GetName() const override {
    return std::string(builtin::MountainMesh);
  }
};

class SphereMesh : public Mesh {
public:
  /** 以共享边中点细分二十面体；最多六级以满足 16 位索引上限。 */
  SphereMesh(float radius = 10, unsigned numSubdivisions = 6);
  /** 返回球体的规范资源名，供 ResourceManager 自动注册。 */
  std::string GetName() const override {
    return std::string(builtin::SphereMesh);
  }
  /** 细分一次并复用相邻三角形的边中点，保持曲面顶点连续。 */
  void Subdivide();
};
} // namespace z8

#undef Def_Mesh
