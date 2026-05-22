//
// Created by zhou_zhengming on 2026/5/21.
//
#include "UI/Material/MaterialRegistry.h"
#include "UI/Material/Material.h"

using namespace z8;

void MaterialRegistry::Register(Material *M) {
  Materials.emplace_back(M);
  Map[M->Name] = M;
}

Material *MaterialRegistry::Get(std::string name) {
  if (!Map.count(name)) return nullptr;
  return Map[name];
}