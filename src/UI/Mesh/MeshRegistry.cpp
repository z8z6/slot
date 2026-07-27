//
// Created by zhou_zhengming on 2026/5/19.
//

#include "UI/Mesh/MeshRegistry.h"
#include "UI/Mesh/RectMesh.h"

using namespace z8;

MeshRegistry::MeshRegistry() = default;

void MeshRegistry::Register(Mesh* M) {
  Meshes.emplace_back(M);
  M->ComputeNormals();
  Map[M->Name] = Meshes.back();
}

Mesh* MeshRegistry::Get(std::string name) {
  if (!Map.count(name)) return nullptr;
  return Map[name];
}