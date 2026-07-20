//
// Created by zhou_zhengming on 2026/5/20.
//

#include "UI/Object/SimpleGameObject.h"
#include "UI/Object/Camera.h"

using namespace z8;
using namespace DirectX;


void SimpleGameObject::Update(Timer* T) {
  // 更新位置
  Transform.UpdateWorld();
  XMMATRIX w = XMLoadFloat4x4(&Transform.World);

  // HLSL默认使用列主序矩阵，DirectXMath 库默认使用行主序矩阵
  // 二者存储顺序相反，必须通过转置统一
  XMStoreFloat4x4(&Const, XMMatrixTranspose(w));
}