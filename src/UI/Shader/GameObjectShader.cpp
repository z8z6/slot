//
// Created by zhou_zhengming on 2026/5/21.
//
#include "Target/DirectX/DX12Shader.h"
#include "UI/Shader/GameObjectShader.h"

using namespace z8;

static DX12ShaderRegister<GameObjectPixelShader> P;
static DX12ShaderRegister<GameObjectVertexShader> V;

z8::GameObjectPixelShader::GameObjectPixelShader()
{
  Name = "GameObject_P";
  FileName = L"shader/GameObject.hlsl";
}

GameObjectVertexShader::GameObjectVertexShader() {
  Name = "GameObject_V";
  FileName = L"shader/GameObject.hlsl";
}