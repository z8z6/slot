//
// Created by zhou_zhengming on 2026/5/17.
//

#include "UI/Object/UIObject.h"

#include "UI/Object/Camera.h"

using namespace DirectX;

void z8::UIObject::Update(Camera* C, Timer* T) {
  Transform.UpdateWorld();
  Transform.UpdateWorldViewProj(C->GetView(), C->GetProj());
  XMMATRIX wvp = XMLoadFloat4x4(&Transform.WorldViewProj);
  XMStoreFloat4x4(&Const, XMMatrixTranspose(wvp));
}