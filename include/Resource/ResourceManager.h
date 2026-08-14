#pragma once

#include "Material/Material.h"
#include "Mesh/Mesh.h"
#include "ResourcePool.h"
#include "Shader/Shader.h"
#include "Shader/ShaderProgram.h"

#include <memory>
#include <string>

namespace z8 {

/**
 * @brief 应用级资源所有者和类型安全查询入口。
 *
 * Manager 本身不是单例：Application 显式拥有它，DX12 等后端只缓存对应的 GPU
 * 表示。这样一个进程可安全创建多个测试上下文，并为未来的多 Device 留出边界。
 */
class ResourceManager {
public:
  /** 创建应用级资源上下文，并按确定顺序注册全部内建资源。 */
  ResourceManager();

  /** 接管有效 Mesh；按其法线策略生成或保留法线，失败时返回无效句柄。 */
  ResourceHandle<Mesh> AddMesh(std::string assetId,
                               std::unique_ptr<Mesh> mesh);
  /** 接管共享 Material 数据，不创建对象私有副本。 */
  ResourceHandle<Material> AddMaterial(std::string assetId,
                                       std::unique_ptr<Material> material);
  /** 接管后端无关的 Shader 编译描述，实际字节码由渲染后端拥有。 */
  ResourceHandle<Shader> AddShader(std::string assetId,
                                   std::unique_ptr<Shader> shader);
  /** 接管完整图形 Program 描述；阶段句柄必须来自同一 Manager。 */
  ResourceHandle<ShaderProgram>
  AddShaderProgram(std::string assetId,
                   std::unique_ptr<ShaderProgram> program);

  /** 把持久化软引用解析为运行时句柄，不执行 I/O。 */
  template <typename ResourceTy>
  ResourceHandle<ResourceTy>
  Resolve(const ResourceReference<ResourceTy>& reference) const;

  /** 仅在句柄仍属于当前槽位 Generation 时返回资源观察指针。 */
  template <typename ResourceTy>
  ResourceTy* TryGet(ResourceHandle<ResourceTy> handle);

  /** const 版本用于后端只读缓存构建。 */
  template <typename ResourceTy>
  const ResourceTy* TryGet(ResourceHandle<ResourceTy> handle) const;

  const ResourcePool<Mesh>& GetMeshes() const { return Meshes; }
  const ResourcePool<Material>& GetMaterials() const { return Materials; }
  const ResourcePool<Shader>& GetShaders() const { return Shaders; }
  const ResourcePool<ShaderProgram>& GetShaderPrograms() const {
    return ShaderPrograms;
  }

private:
  void RegisterBuiltinResources();

  ResourcePool<Mesh> Meshes;
  ResourcePool<Material> Materials;
  ResourcePool<Shader> Shaders;
  ResourcePool<ShaderProgram> ShaderPrograms;
};

template <>
inline ResourceHandle<Mesh>
ResourceManager::Resolve(const ResourceReference<Mesh>& reference) const {
  return Meshes.Find(reference.GetAssetId());
}

template <>
inline ResourceHandle<Material>
ResourceManager::Resolve(const ResourceReference<Material>& reference) const {
  return Materials.Find(reference.GetAssetId());
}

template <>
inline ResourceHandle<Shader>
ResourceManager::Resolve(const ResourceReference<Shader>& reference) const {
  return Shaders.Find(reference.GetAssetId());
}

template <>
inline ResourceHandle<ShaderProgram> ResourceManager::Resolve(
    const ResourceReference<ShaderProgram>& reference) const {
  return ShaderPrograms.Find(reference.GetAssetId());
}

template <>
inline Mesh* ResourceManager::TryGet(ResourceHandle<Mesh> handle) {
  return Meshes.TryGet(handle);
}

template <>
inline Material* ResourceManager::TryGet(ResourceHandle<Material> handle) {
  return Materials.TryGet(handle);
}

template <>
inline Shader* ResourceManager::TryGet(ResourceHandle<Shader> handle) {
  return Shaders.TryGet(handle);
}

template <>
inline ShaderProgram*
ResourceManager::TryGet(ResourceHandle<ShaderProgram> handle) {
  return ShaderPrograms.TryGet(handle);
}

template <>
inline const Mesh* ResourceManager::TryGet(ResourceHandle<Mesh> handle) const {
  return Meshes.TryGet(handle);
}

template <>
inline const Material*
ResourceManager::TryGet(ResourceHandle<Material> handle) const {
  return Materials.TryGet(handle);
}

template <>
inline const Shader*
ResourceManager::TryGet(ResourceHandle<Shader> handle) const {
  return Shaders.TryGet(handle);
}

template <>
inline const ShaderProgram*
ResourceManager::TryGet(ResourceHandle<ShaderProgram> handle) const {
  return ShaderPrograms.TryGet(handle);
}

} // namespace z8
