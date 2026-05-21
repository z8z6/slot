//
// Created by zhou_zhengming on 2026/5/20.
//

#include "UI/Object/SimpleGameObject.h"
#include "UI/Object/Camera.h"

using namespace z8;
using namespace DirectX;


void SimpleGameObject::Update(Camera* C, Timer* T) {
  Transform.UpdateWorld();
  Transform.UpdateWorldViewProj(C->GetView(), C->GetProj());
  XMMATRIX wvp = XMLoadFloat4x4(&Transform.WorldViewProj);
  XMStoreFloat4x4(&Const, XMMatrixTranspose(wvp));
}