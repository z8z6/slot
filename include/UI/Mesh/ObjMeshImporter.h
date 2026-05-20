//
// Created by zhou_zhengming on 2026/5/20.
//

#pragma once
#include "Mesh.h"
#include <string>

namespace z8 {
class ObjMeshImporter {
public:
  static Mesh Parse(std::string FileName);
};
}





