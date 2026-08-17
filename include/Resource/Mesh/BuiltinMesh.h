//
// Created by zhou_zhengming on 2026/8/15.
//

#pragma once
#include "Mesh.h"

#include "Resource/BuiltinResource.h"

namespace z8 {
struct CubeMesh : Mesh {
  CubeMesh();
};

struct RectMesh : Mesh {
  RectMesh();
};

struct GridMesh : Mesh {
  GridMesh();
};

struct SkullMesh : Mesh {
  SkullMesh();
};

struct TriangleMesh : Mesh {
  TriangleMesh();
};

class MountainMesh : public GridMesh {
public:
  MountainMesh();
  float GetHeight(float x, float z) const;
};

class SphereMesh : public Mesh {
public:
  /** 以共享边中点细分二十面体；最多六级以满足 16 位索引上限。 */
  SphereMesh(float radius = 10, unsigned numSubdivisions = 6);
  /** 细分一次并复用相邻三角形的边中点，保持曲面顶点连续。 */
  void Subdivide();
};
} // namespace z8

