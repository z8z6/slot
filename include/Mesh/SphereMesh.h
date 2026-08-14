//
// Created by zhou_zhengming on 2026/5/19.
//

#pragma once
#include "Mesh.h"

namespace z8 {
class SphereMesh : public Mesh{
public:
  /** 以共享边中点细分二十面体；最多六级以满足 16 位索引上限。 */
  SphereMesh(float radius = 10, unsigned numSubdivisions = 6);
  /** 细分一次并复用相邻三角形的边中点，保持曲面顶点连续。 */
  void Subdivide();
};
} // namespace z8



