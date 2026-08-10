//
// Created by zhou_zhengming on 2026/5/17.
//

#include "UI/Object/UIObject/UIObject.h"

#include "UI/Object/Camera/Camera.h"
#include "Target/DirectX/DX12Shader.h"
#include "UI/Shader/UIObjectShader.h"

#include <iostream>
#include <ostream>

using namespace DirectX;
using namespace z8;

z8::UIObject::UIObject() {
  PixelShader = DX12ShaderRegistry::Instance().Get("UIObject_P");
  VertexShader = DX12ShaderRegistry::Instance().Get("UIObject_V");
}

void UIObject::SetPosition(float x, float y, float w, float h) {
  // 默认网格原点在中心，且变长为 0.5
  // 将左上角坐标转为中心坐标
  Transform.Position.x = x + w / 2;
  Transform.Position.y = y + h / 2;
}

void UIObject::SetScale(float x, float y) {
  // 默认原网格总长度为 1
  Transform.Scale.x = x;
  Transform.Scale.y = y;
}

void z8::UIObject::Update(Timer* T) {
  Transform.UpdateWorld();
  XMMATRIX w = XMLoadFloat4x4(&Transform.World);
  XMStoreFloat4x4(&Const.World, XMMatrixTranspose(w));
}
