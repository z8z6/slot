//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once

#include "Vertex.h"
#include <vector>

/**
 * 描述物体顶点
 * 1. 从相机视角，顶点绕序决定正反面，顺时针为正面，逆时针为背面
 */

namespace z8 {
class Mesh {
public:
  using IndexTy = uint16_t;
  std::vector<Vertex> V;
  std::vector<IndexTy> I;

  Mesh() = default;
  virtual ~Mesh() = default;

  unsigned VSize() const { return V.size() * sizeof(Vertex); }
  unsigned VElemSize() const { return sizeof(Vertex); }
  unsigned ISize() const { return I.size() * sizeof(IndexTy); }
  unsigned ICount() const { return I.size(); }

};
}
