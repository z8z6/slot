//
// Created by zhou_zhengming on 2026/5/19.
//

#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace z8 {
class Mesh;
class MeshRegistry {
  std::unordered_map<std::string, Mesh *> Map;
  MeshRegistry();
public:
  std::vector<Mesh*> Meshes;

  MeshRegistry(const MeshRegistry&) = delete;
  MeshRegistry& operator=(const MeshRegistry&) = delete;
  MeshRegistry(MeshRegistry&&) = delete;
  MeshRegistry& operator=(MeshRegistry&&) = delete;

  void Register(Mesh*);
  Mesh* GetMesh(std::string name);

  static MeshRegistry &Instance() {
    static MeshRegistry instance;
    return instance;
  }
};

template <typename MeshTy>
class MeshRegister {
public:
  MeshRegister() {
    MeshRegistry::Instance().Register(new MeshTy());
  }
};

}
