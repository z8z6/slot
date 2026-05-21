//
// Created by zhou_zhengming on 2026/5/20.
//

#include "UI/Mesh/AmiyaMesh.h"

#include "UI/Mesh/MeshRegistry.h"
#include "UI/Mesh/ObjMeshImporter.h"

using namespace z8;

// static MeshRegister<AmiyaMesh> R;

z8::AmiyaMesh::AmiyaMesh() {
  Mesh M = ObjMeshImporter::Parse("mesh/Amiya/Amiya.obj");
  V = M.V;
  I = M.I;
  Name = "Amiya";
}