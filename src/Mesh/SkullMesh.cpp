//
// Created by zhou_zhengming on 2026/5/20.
//

#include "Mesh/SkullMesh.h"
#include "Mesh/ObjMeshImporter.h"

using namespace z8;

SkullMesh::SkullMesh() {
  Mesh M = ObjMeshImporter::Parse("mesh/Skull/Skull.obj");
  V = M.V;
  I = M.I;
  Name = "Skull";
}
