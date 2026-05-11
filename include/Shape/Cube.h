//
// Created by zhou_zhengming on 2026/5/11.
//
#pragma once

#include "IShape.h"

namespace z8 {
class Cube : public IShape {
private:
  std::vector<Vertex> Vs;
  std::vector<VertexGroup> Is;
  Constant Const;

public:
  Cube();

  std::vector<Vertex> &V() override { return Vs; }
  std::vector<VertexGroup> &I() override { return Is; }
  Constant &C() override { return Const; };
};
}