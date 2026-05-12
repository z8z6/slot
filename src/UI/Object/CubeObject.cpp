//
// Created by zhou_zhengming on 2026/5/12.
//

#include "UI/Object/CubeObject.h"
#include "UI/Material/DefaultMaterial.h"
#include "UI/Mesh/CubeMesh.h"

z8::CubeObject::CubeObject()
{
  Mesh = new CubeMesh;
  Material = new DefaultMaterial;
  WorldViewProj = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};
}
