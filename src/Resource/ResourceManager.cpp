#include "Resource/ResourceManager.h"
#include "Material/BuiltinMaterial.h"
#include "Mesh/BuiltinMesh.h"
#include "Resource/BuiltinResource.h"
#include "Shader/BuiltinShader.h"
#include "Texture/BuiltinTexture.h"

using namespace z8;

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

  // 先注册阶段再构造 Program：Program 保存的是类型化运行时索引引用，
  // 这个顺序保证它们在进入 PSO 缓存前已经稳定。内建类则是
  // ID、编译入口和固定管线状态的唯一来源。
  const auto gameObjectVertex = Add(std::make_unique<GameObjectVertexShader>());
  const auto gameObjectPixel = Add(std::make_unique<GameObjectPixelShader>());
  Add(std::make_unique<GameObjectShader>(gameObjectVertex, gameObjectPixel));

  const auto missingVertex = Add(std::make_unique<MissingVertexShader>());
  const auto missingPixel = Add(std::make_unique<MissingPixelShader>());
  Add(std::make_unique<MissingShader>(missingVertex, missingPixel));


  const auto uiObjectVertex = Add(std::make_unique<UIObjectVertexShader>());
  const auto uiObjectPixel = Add(std::make_unique<UIObjectPixelShader>());
  Add(std::make_unique<UIObjectShader>(uiObjectVertex, uiObjectPixel));
}
