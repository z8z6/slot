//
// Created by zhou_zhengming on 2026/5/12.
//

#include "UI/Object/CubeObject.h"
#include "UI/Mesh/CubeMesh.h"
#include "UI/Mesh/MeshRegistry.h"

using namespace DirectX;

void z8::CubeObject::Update(const XMFLOAT4X4& View, const XMFLOAT4X4& Proj)
{
  Transform.UpdateWorld();
  Transform.UpdateWorldViewProj(View, Proj);
  XMMATRIX wvp = XMLoadFloat4x4(&Transform.WorldViewProj);
  XMStoreFloat4x4(&objConstants, XMMatrixTranspose(wvp));
}

void* z8::CubeObject::ConstBuf()
{
  return &objConstants;
}

unsigned z8::CubeObject::ConstBufSize()
{
  return sizeof(objConstants);
}

z8::CubeObject::CubeObject()
{
  Mesh = MeshRegistry::Instance().GetMesh("Cube");
}
