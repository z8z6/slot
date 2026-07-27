//
// Created by zhou_zhengming on 2026/7/27.
//

#include "UI/Shader/UIObjectShader.h"
#include "Target/DirectX/DX12Shader.h"

using namespace z8;

static DX12ShaderRegister<UIObjectPixelShader> P;
static DX12ShaderRegister<UIObjectVertexShader> V;

UIObjectPixelShader::UIObjectPixelShader() {
  Name = "UIObject_P";
  FileName = L"shader/UIObject.hlsl";
}

UIObjectVertexShader::UIObjectVertexShader() {
  Name = "UIObject_V";
  FileName = L"shader/UIObject.hlsl";
}