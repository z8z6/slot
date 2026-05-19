//
// Created by zhou_zhengming on 2026/5/19.
//

#include "UI/Object/GridObject.h"
#include "UI/Mesh/MeshRegistry.h"

using namespace z8;
using namespace DirectX;

void GridObject::Update(const XMFLOAT4X4 &View,
                        const XMFLOAT4X4 &Proj) {
  Transform.UpdateWorld();
  Transform.UpdateWorldViewProj(View, Proj);
  XMMATRIX wvp = XMLoadFloat4x4(&Transform.WorldViewProj);
  XMStoreFloat4x4(&objConstants, XMMatrixTranspose(wvp));
}

GridObject::GridObject() {
  Mesh = MeshRegistry::Instance().GetMesh("Grid");
}

void *GridObject::ConstBuf() {
  return &objConstants;
}

unsigned GridObject::ConstBufSize() {
  return sizeof(objConstants);
}