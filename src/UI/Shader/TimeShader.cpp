//
// Created by zhou_zhengming on 2026/5/21.
//

#include "UI/Shader/TimeShader.h"
#include "Target/DirectX/DX12Shader.h"

using namespace z8;

static DX12ShaderRegister<TimeShader> R;

TimeShader::TimeShader() {
  Name = "Time";
  FileName = L"shader/Time.hlsl";
}