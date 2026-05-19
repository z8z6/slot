//
// Created by zhou_zhengming on 2026/5/19.
//

#include "UI/Mesh/MeshRegistry.h"
#include "UI/Mesh/CubeMesh.h"
#include "UI/Mesh/RectangleMesh.h"

using namespace z8;

MeshRegistry::MeshRegistry() {
  Prepare();
}

void MeshRegistry::Prepare() {
  Meshes.emplace_back(new CubeMesh());
  Map["Cube"] = Meshes.back();
  Meshes.emplace_back(new RectangleMesh());
  Map["Rectangle"] = Meshes.back();
}

Mesh* MeshRegistry::GetMesh(std::string name) {
  if (!Map.count(name)) return nullptr;
  return Map[name];
}