//
// Created by zhou_zhengming on 2026/5/21.
//
#include "UI/Shader/DefaultShader.h"
#include "Target/DirectX/DX12Shader.h"

using namespace z8;

static DX12ShaderRegister<DefaultShader> R;

z8::DefaultShader::DefaultShader()
{
  Name = "Default";
  FileName = L"shader/Default.hlsl";
}