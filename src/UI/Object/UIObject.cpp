//
// Created by zhou_zhengming on 2026/5/17.
//

#include "UI/Object/UIObject.h"

#include "UI/Object/Camera.h"

using namespace DirectX;

void z8::UIObject::Update(Timer* T) {
  Transform.UpdateWorld();
  XMMATRIX w = XMLoadFloat4x4(&Transform.World);
  XMStoreFloat4x4(&Const, XMMatrixTranspose(w));
}