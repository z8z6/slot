//
// Created by zhou_zhengming on 2026/5/17.
//

#include "UI/Object/GameObject.h"

#include "Target/DirectX/DX12Shader.h"
#include "UI/Material/MetalMaterial.h"

z8::GameObject::GameObject(): Mesh(nullptr), Collider(nullptr), Material(new MetalMaterial())
{
  PixelShader = DX12ShaderRegistry::Instance().Get("Default_P");
  VertexShader = DX12ShaderRegistry::Instance().Get("Default_V");
}
