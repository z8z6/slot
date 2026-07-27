//
// Created by zhou_zhengming on 2026/5/17.
//

#include "../../../../include/UI/Object/UIObject/UIObject.h"

#include "../../../../include/UI/Object/Camera/Camera.h"
#include "Target/DirectX/DX12Shader.h"
#include "UI/Shader/UIObjectShader.h"

using namespace DirectX;

z8::UIObject::UIObject() {
  PixelShader = DX12ShaderRegistry::Instance().Get(UIObjectPixelShader().Name);
  VertexShader = DX12ShaderRegistry::Instance().Get(UIObjectVertexShader().Name);
}

void z8::UIObject::Update(Timer* T) {
  Transform.UpdateWorld();
  XMMATRIX w = XMLoadFloat4x4(&Transform.World);
  XMStoreFloat4x4(&Const, XMMatrixTranspose(w));
}