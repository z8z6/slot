//
// Created by zhou_zhengming on 2026/5/19.
//

#pragma once
#include "Mesh.h"

namespace z8 {
class GridMesh : public Mesh{
public:
  GridMesh(float width = 20, float depth = 20, unsigned m = 5, unsigned n = 5);
};

}




