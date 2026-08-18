//
// Created by zhou_zhengming on 2026/8/15.
//

#pragma once
#include "BaseMesh.h"

namespace z8 {
struct TriangleMesh : BaseMesh {
  TriangleMesh();
};

struct RectMesh : BaseMesh {
  RectMesh();
};

struct GridMesh : BaseMesh {
  float LengthX = 1;
  float LengthZ = 1;
  unsigned DivX = 5;
  unsigned DivZ = 5;

  GridMesh();
  void Update() override;
};

struct CubeMesh : BaseMesh {
  CubeMesh();
};

struct SkullMesh : BaseMesh {
  SkullMesh();
};

struct  MountainMesh : GridMesh {
  using HeightFunc = float (*)(float, float);
  HeightFunc Func;

  MountainMesh();
  void Update() override;
};

struct SphereMesh : BaseMesh {
  float radius = 10;
  unsigned numSubdivisions = 6;

  SphereMesh();
  void Subdivide();
  void Update() override;
};
} // namespace z8

