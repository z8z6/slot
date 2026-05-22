//
// Created by zhou_zhengming on 2026/5/21.
//

#pragma once
#include <string>
#include <unordered_map>

namespace z8 {
class Material;
class MaterialRegistry {
  std::unordered_map<std::string, Material *> Map;
  MaterialRegistry() = default;
public:
  std::vector<Material*> Materials;

  MaterialRegistry(const MaterialRegistry&) = delete;
  MaterialRegistry& operator=(const MaterialRegistry&) = delete;
  MaterialRegistry(MaterialRegistry&&) = delete;
  MaterialRegistry& operator=(MaterialRegistry&&) = delete;

  void Register(Material*);
  Material* Get(std::string name);

  static MaterialRegistry &Instance() {
    static MaterialRegistry instance;
    return instance;
  }
};

template <typename MaterialTy>
class MaterialRegister {
public:
  MaterialRegister() {
    MaterialRegistry::Instance().Register(new MaterialTy());
  }
};
}




