//
// Created by zhou_zhengming on 2026/5/21.
//

#include "UI/Shader/MissingShader.h"
#include "Target/DirectX/DX12Shader.h"

using namespace z8;

static DX12ShaderRegister<MissingShader> R;

z8::MissingShader::MissingShader() {
  Name = "Missing";
  FileName = L"shader/Missing.hlsl";
}