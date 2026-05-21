//
// Created by zhou_zhengming on 2026/5/21.
//

#include "UI/Shader/TimeShader.h"
#include "Target/DirectX/DX12Shader.h"

using namespace z8;

static DX12ShaderRegister<TimePixelShader> P;
static DX12ShaderRegister<TimeVertexShader> V;

TimePixelShader::TimePixelShader() {
  Name = "Time_P";
  FileName = L"shader/Time.hlsl";
}

TimeVertexShader::TimeVertexShader() {
  Name = "Time_V";
  FileName = L"shader/Time.hlsl";
}