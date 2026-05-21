//
// Created by zhou_zhengming on 2026/5/21.
//

#pragma once
#include "GridMesh.h"

namespace z8 {
class MountainMesh : public GridMesh{
public:
  MountainMesh(float width = 100, float depth = 100, unsigned m = 101, unsigned n = 101);
  float GetHeight(float x, float z) const;
};

}



