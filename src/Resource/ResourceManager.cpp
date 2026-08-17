#include "Resource/ResourceManager.h"
#include "Material/BuiltinMaterial.h"
#include "Mesh/BuiltinMesh.h"
#include "Resource/BuiltinResource.h"
#include "Texture/BuiltinTexture.h"

using namespace z8;

ResourceManager::ResourceManager() { RegisterBuiltinResources(); }

void ResourceManager::RegisterBuiltinResources() {
  // 显式启动注册替代跨翻译单元静态构造，注册顺序和所有权因此完全可观察。
  Add(std::make_unique<CubeMesh>());
  Add(std::make_unique<GridMesh>());
  Add(std::make_unique<MountainMesh>());
  Add(std::make_unique<RectMesh>());
  Add(std::make_unique<SkullMesh>());
  Add(std::make_unique<SphereMesh>());
  Add(std::make_unique<GrassBlockTexture>());
  Add(std::make_unique<GrassBlockMaterial>());
  Add(std::make_unique<MetalMaterial>());
  Add(std::make_unique<UIMaterial>());

  // 阶段描述在代码中显式建立；先注册 Shader 再构造 Program，保证句柄在进入
  // 渲染批缓存前已经稳定，并避免构建期生成类成为资源身份的隐藏来源。
  const auto gameObjectVertex = Add(std::make_unique<Shader>(
      builtin::shader::GameObjectVertex, L"asset/shader/GameObject.hlsl",
      "GameObject_V", "vs_6_0", "VS"));
  const auto gameObjectPixel = Add(std::make_unique<Shader>(
      builtin::shader::GameObjectPixel, L"asset/shader/GameObject.hlsl",
      "GameObject_P", "ps_6_0", "PS"));
  Add(std::make_unique<ShaderProgram>(
      builtin::shader::program::GameObjectProgram, gameObjectVertex,
      gameObjectPixel, true, false));

  const auto missingVertex = Add(std::make_unique<Shader>(
      builtin::shader::MissingVertex, L"asset/shader/Missing.hlsl", "Missing_V",
      "vs_6_0", "VS"));
  const auto missingPixel = Add(std::make_unique<Shader>(
      builtin::shader::MissingPixel, L"asset/shader/Missing.hlsl", "Missing_P",
      "ps_6_0", "PS"));
  Add(std::make_unique<ShaderProgram>(builtin::shader::program::MissingProgram,
                                      missingVertex, missingPixel, true,
                                      false));

  const auto timeVertex = Add(std::make_unique<Shader>(
      builtin::shader::TimeVertex, L"asset/shader/Time.hlsl", "Time_V",
      "vs_6_0", "VS"));
  const auto timePixel = Add(std::make_unique<Shader>(
      builtin::shader::TimePixel, L"asset/shader/Time.hlsl", "Time_P", "ps_6_0",
      "PS"));
  Add(std::make_unique<ShaderProgram>(builtin::shader::program::TimeProgram,
                                      timeVertex, timePixel, true, false));

  const auto uiObjectVertex = Add(std::make_unique<Shader>(
      builtin::shader::UIObjectVertex, L"asset/shader/UIObject.hlsl",
      "UIObject_V", "vs_6_0", "VS"));
  const auto uiObjectPixel = Add(std::make_unique<Shader>(
      builtin::shader::UIObjectPixel, L"asset/shader/UIObject.hlsl",
      "UIObject_P", "ps_6_0", "PS"));
  Add(std::make_unique<ShaderProgram>(builtin::shader::program::UIObjectProgram,
                                      uiObjectVertex, uiObjectPixel, false,
                                      true));
}
