//
// Created by zhou_zhengming on 2026/5/20.
//

#pragma once
#include "BaseMesh.h"
#include <string>

namespace z8 {
class ObjImporter : MeshImporter {
public:
  static BaseMesh Parse(std::string FileName);
};
}





