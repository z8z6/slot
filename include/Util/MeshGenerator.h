//
// Created by zhou_zhengming on 2026/5/20.
//

#pragma once

#include "Mesh/Vertex.h"

namespace z8 {
class MeshGenerator {
public:
  static Vertex MidPoint(const Vertex& v0, const Vertex& v1);
};

}
