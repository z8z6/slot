//
// Created by zhou_zhengming on 2026/5/11.
//

#pragma once

#include "Vertex.h"
#include "Constant.h"
#include <vector>

namespace z8 {
class IShape {
public:
  virtual std::vector<Vertex>&      V() = 0;
  virtual std::vector<VertexGroup>& I() = 0;
  virtual Constant& C() = 0;

  virtual ~IShape() = default;
  unsigned VSize() { return V().size() * sizeof(Vertex); }
  unsigned VElemSize() { return sizeof(Vertex); }
  unsigned ISize() { return I().size() * sizeof(VertexGroup); }
  unsigned ICount() { return I().size() * 3; }
  unsigned CSize() { return sizeof(Constant); }
};
}