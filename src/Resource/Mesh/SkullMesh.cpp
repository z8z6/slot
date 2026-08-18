//
// Created by zhou_zhengming on 2026/5/20.
//

#include "Mesh/BuiltinMesh.h"
#include "Mesh/ObjImporter.h"

using namespace z8;

SkullMesh::SkullMesh() {
  Id = builtin::mesh::SkullMesh;
  BaseMesh M = ObjImporter::Parse("asset/mesh/Skull/Skull.obj");
  V = M.V;
  I = M.I;
}
