//
// Created by zhou_zhengming on 2026/5/17.
//

#include "UI/Object/GameObject.h"

#include "Target/DirectX/DX12Shader.h"
#include "UI/Material/DefaultMaterial.h"

z8::GameObject::GameObject(): Mesh(nullptr), Collider(nullptr), Material(new DefaultMaterial())
{
  PixelShader = DX12ShaderRegistry::Instance().GetShader("Default_P");
  VertexShader = DX12ShaderRegistry::Instance().GetShader("Default_V");
}
