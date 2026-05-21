//
// Created by zhou_zhengming on 2026/5/19.
//

#pragma once
#include "Mesh.h"

namespace z8 {
class GridMesh : public Mesh{
public:
  GridMesh(float width = 100, float depth = 100, unsigned m = 101, unsigned n = 101);
};

}




