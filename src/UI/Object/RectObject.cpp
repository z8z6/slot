//
// Created by zhou_zhengming on 2026/5/15.
//
#include "UI/Object/RectObject.h"

#include "UI/Mesh/MeshRegistry.h"
#include "UI/Mesh/RectangleMesh.h"

using namespace z8;
using namespace DirectX;

void* RectObject::ConstBuf()
{
  return &objConstants;
}

unsigned RectObject::ConstBufSize()
{
  return sizeof(objConstants);
}

void RectObject::Update(const DirectX::XMFLOAT4X4& View, const DirectX::XMFLOAT4X4& Proj)
{
  Transform.UpdateWorldViewProj(View, Proj);
  XMMATRIX wvp = XMLoadFloat4x4(&Transform.WorldViewProj);
  XMStoreFloat4x4(&objConstants, XMMatrixTranspose(wvp));
}

RectObject::RectObject()
{
  Mesh = MeshRegistry::Instance().GetMesh("Rectangle");
}
