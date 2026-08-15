#include "Resource/ResourceManager.h"
#include "Material/BuiltinMaterial.h"
#include "Mesh/BuiltinMesh.h"

#include <stdexcept>

using namespace z8;

namespace z8 {
// 构建期生成函数提供显式注册入口，避免静态初始化顺序和链接裁剪问题。
void RegisterGeneratedShaders(ResourceManager& resources);
}

ResourceManager::ResourceManager() { RegisterBuiltinResources(); }

void ResourceManager::RegisterBuiltinResources() {
  // 显式启动注册替代跨翻译单元静态构造，注册顺序和所有权因此完全可观察。
  Add(std::make_unique<CubeMesh>());
  Add(std::make_unique<GridMesh>());
  Add(std::make_unique<MountainMesh>());
  Add(std::make_unique<RectMesh>());
  Add(std::make_unique<SkullMesh>());
  Add(std::make_unique<SphereMesh>());
  auto grassTexture = std::make_unique<Texture>();
  grassTexture->AssetId = builtin::GrassBlockTexture;
  std::string textureError;
  if (!grassTexture->Load(L"asset/texture/grass-block.png", &textureError))
    throw std::runtime_error(textureError);
  Add(std::move(grassTexture));
  Add(std::make_unique<GrassBlockMaterial>());
  Add(std::make_unique<MetalMaterial>());
  Add(std::make_unique<UIMaterial>());

  RegisterGeneratedShaders(*this);
}
