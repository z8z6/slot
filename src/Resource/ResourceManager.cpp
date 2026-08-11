#include "Resource/ResourceManager.h"

#include "Material/MetalMaterial.h"
#include "Mesh/CubeMesh.h"
#include "Mesh/GridMesh.h"
#include "Mesh/MountainMesh.h"
#include "Mesh/RectMesh.h"
#include "Mesh/SkullMesh.h"
#include "Mesh/SphereMesh.h"
#include "Resource/BuiltinResource.h"

#include <utility>

using namespace z8;

namespace z8 {
// 构建期生成函数提供显式注册入口，避免静态初始化顺序和链接裁剪问题。
void RegisterGeneratedShaders(ResourceManager& resources);
}

ResourceManager::ResourceManager() { RegisterBuiltinResources(); }

ResourceHandle<Mesh> ResourceManager::AddMesh(std::string assetId,
                                               std::unique_ptr<Mesh> mesh) {
  if (mesh) mesh->ComputeNormals();
  return Meshes.Add(std::move(assetId), std::move(mesh));
}

ResourceHandle<Material>
ResourceManager::AddMaterial(std::string assetId,
                             std::unique_ptr<Material> material) {
  return Materials.Add(std::move(assetId), std::move(material));
}

ResourceHandle<Shader>
ResourceManager::AddShader(std::string assetId,
                           std::unique_ptr<Shader> shader) {
  return Shaders.Add(std::move(assetId), std::move(shader));
}

ResourceHandle<ShaderProgram> ResourceManager::AddShaderProgram(
    std::string assetId, std::unique_ptr<ShaderProgram> program) {
  return ShaderPrograms.Add(std::move(assetId), std::move(program));
}

void ResourceManager::RegisterBuiltinResources() {
  // 显式启动注册替代跨翻译单元静态构造，注册顺序和所有权因此完全可观察。
  AddMesh(std::string(builtin::CubeMesh), std::make_unique<CubeMesh>());
  AddMesh(std::string(builtin::GridMesh), std::make_unique<GridMesh>());
  AddMesh(std::string(builtin::MountainMesh), std::make_unique<MountainMesh>());
  AddMesh(std::string(builtin::RectMesh), std::make_unique<RectMesh>());
  AddMesh(std::string(builtin::SkullMesh), std::make_unique<SkullMesh>());
  AddMesh(std::string(builtin::SphereMesh), std::make_unique<SphereMesh>());
  AddMaterial(std::string(builtin::MetalMaterial),
              std::make_unique<MetalMaterial>());
  auto uiMaterial = std::make_unique<Material>();
  uiMaterial->Name = "UI";
  uiMaterial->Program =
      ResourceReference<ShaderProgram>(builtin::UIObjectProgram);
  AddMaterial(std::string(builtin::UIMaterial), std::move(uiMaterial));

  RegisterGeneratedShaders(*this);
}
