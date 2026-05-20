//
// Created by zhou_zhengming on 2026/5/19.
//

#pragma once
#include "Mesh.h"

namespace z8 {
class SphereMesh : public Mesh{
public:
  SphereMesh(float radius = 10, unsigned numSubdivisions = 6);
  void Subdivide();
};
}



