//
// Created by zhou_zhengming on 2026/5/20.
//

#include "UI/Object/SimpleGameObject.h"

using namespace z8;
using namespace DirectX;


void SimpleGameObject::Update(const XMFLOAT4X4 &View,
                              const XMFLOAT4X4 &Proj) {
  Transform.UpdateWorld();
  Transform.UpdateWorldViewProj(View, Proj);
  XMMATRIX wvp = XMLoadFloat4x4(&Transform.WorldViewProj);
  XMStoreFloat4x4(&Const, XMMatrixTranspose(wvp));
}