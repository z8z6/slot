//
// Created by zhou_zhengming on 2026/5/19.
//

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace z8 {
class Mesh;
class MeshRegistry {
  std::unordered_map<std::string, Mesh *> Map;
public:
  std::vector<Mesh*> Meshes;

  MeshRegistry();

  void Prepare();
  Mesh* GetMesh(std::string name);

  static MeshRegistry &Instance() {
    static MeshRegistry instance;
    return instance;
  }
};
}
