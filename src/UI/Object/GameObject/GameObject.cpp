//
// Created by zhou_zhengming on 2026/5/17.
//

#include "UI/Object/GameObject/GameObject.h"
#include "UI/Shader/GameObjectShader.h"
#include "Target/DirectX/DX12Shader.h"
#include "UI/Material/MetalMaterial.h"

z8::GameObject::GameObject()
    : Mesh(nullptr), Material(nullptr), Collider(nullptr),
      OwnedMaterial(std::make_unique<MetalMaterial>())
{
  Material = OwnedMaterial.get();
  // 控件/场景对象只依赖稳定资源名，不为查询名称临时构造 Shader 描述对象。
  PixelShader = DX12ShaderRegistry::Instance().Get("GameObject_P");
  VertexShader = DX12ShaderRegistry::Instance().Get("GameObject_V");
}

z8::GameObject::~GameObject() = default;
