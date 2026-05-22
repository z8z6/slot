//
// Created by zhou_zhengming on 2026/5/21.
//
#include "UI/Shader/DefaultShader.h"
#include "Target/DirectX/DX12Shader.h"

using namespace z8;

static DX12ShaderRegister<DefaultPixelShader> P;
static DX12ShaderRegister<DefaultVertexShader> V;

z8::DefaultPixelShader::DefaultPixelShader()
{
  Name = "Default_P";
  FileName = L"shader/Default.hlsl";
}

DefaultVertexShader::DefaultVertexShader() {
  Name = "Default_V";
  FileName = L"shader/Default.hlsl";
}