//
// Created by zhou_zhengming on 2026/5/20.
//

#include "UI/Mesh/SkullMesh.h"
#include "UI/Mesh/MeshRegistry.h"
#include "UI/Mesh/ObjMeshImporter.h"

using namespace z8;

static MeshRegister<SkullMesh> R;

SkullMesh::SkullMesh() {
  Mesh M = ObjMeshImporter::Parse("mesh/Skull/Skull.obj");
  V = M.V;
  I = M.I;
  Name = "Skull";
}