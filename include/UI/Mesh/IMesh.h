//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once

#include "Vertex.h"
#include <vector>

namespace z8 {
class IMesh {
public:
  std::vector<Vertex> V;
  std::vector<VertexGroup> I;

  IMesh() = default;
  virtual ~IMesh() = default;

  unsigned VSize() { return V.size() * sizeof(Vertex); }
  unsigned VElemSize() { return sizeof(Vertex); }
  unsigned ISize() { return I.size() * sizeof(VertexGroup); }
  unsigned ICount() { return I.size() * 3; }

};
}
