//
// Created by zhou_zhengming on 2026/5/21.
//

#include "UI/Shader/MissingShader.h"
#include "Target/DirectX/DX12Shader.h"

using namespace z8;

static DX12ShaderRegister<MissingPixelShader> P;
static DX12ShaderRegister<MissingVertexShader> V;

z8::MissingPixelShader::MissingPixelShader() {
  Name = "Missing_P";
  FileName = L"shader/Missing.hlsl";
}

z8::MissingVertexShader::MissingVertexShader() {
  Name = "Missing_V";
  FileName = L"shader/Missing.hlsl";
}