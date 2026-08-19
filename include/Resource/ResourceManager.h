#pragma once

#include "Material/BaseMaterial.h"
#include "Mesh/BaseMesh.h"
#include "ResourcePool.h"
#include "Shader/BaseShader.h"
#include "Shader/BaseShaderComponent.h"
#include "Texture/BaseTexture.h"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace z8 {

/** 把具体内建派生类归一到 ResourceManager 实际存储的资源基类。 */
template <typename T>
using StoredResourceTy = std::conditional_t<
    std::is_base_of_v<BaseMesh, T>, BaseMesh,
    std::conditional_t<
        std::is_base_of_v<BaseMaterial, T>, BaseMaterial,
        std::conditional_t<
            std::is_base_of_v<BaseShaderComponent, T>, BaseShaderComponent,
            std::conditional_t<
                std::is_base_of_v<BaseShader, T>, BaseShader,
                std::conditional_t<std::is_base_of_v<BaseTexture, T>,
                                   BaseTexture, void>>>>>;

/**
 * @brief 应用级资源所有者和类型安全查询入口。
 * Manager 本身不是单例：Application 显式拥有它
 */
class ResourceManager {
public:
  ResourcePool<BaseMesh> Meshes;
  ResourcePool<BaseMaterial> Materials;
  ResourcePool<BaseShaderComponent> Shaders;
  ResourcePool<BaseShader> ShaderPrograms;
  ResourcePool<BaseTexture> Textures;

  ResourceManager() { RegisterBuiltinResources(); }

  template <typename T>
  ResourceRef<StoredResourceTy<T>> Add(std::unique_ptr<T> resource);

  /**
   * 使用加载边界提供的 ID 接管资源；适用于 FBX 等名称不属于 builtin URI
   * 的资源。
   */
  template <typename T>
  ResourceRef<StoredResourceTy<T>> Add(std::string assetId,
                                       std::unique_ptr<T> resource);

  /** 把携带 Asset ID 的引用解析为携带稳定 Index 的运行时引用，不执行 I/O。 */
  template <typename ResourceTy>
  ResourceRef<ResourceTy>
  Resolve(const ResourceRef<ResourceTy> &reference) const;

  /** 仅在引用携带当前资源池的有效索引时返回资源观察指针。 */
  template <typename ResourceTy>
  ResourceTy *TryGet(ResourceRef<ResourceTy> reference);

  /** const 版本用于后端只读缓存构建。 */
  template <typename ResourceTy>
  const ResourceTy *TryGet(ResourceRef<ResourceTy> reference) const;

private:
  /** 根据规范名称类别校验自动注册的 C++ 资源类型。 */
  template <typename T> static bool MatchesType(ResourceTy type);

  /** 返回指定资源基类唯一对应的类型池，集中维护类型到存储的映射。 */
  template <typename T> ResourcePool<T> &Pool();

  /** const 池访问供 Resolve 和只读后端使用。 */
  template <typename T> const ResourcePool<T> &Pool() const;

  void RegisterBuiltinResources();
};

template <typename T>
ResourceRef<StoredResourceTy<T>>
ResourceManager::Add(std::unique_ptr<T> resource) {
  using StoredTy = StoredResourceTy<T>;
  static_assert(!std::is_void_v<StoredTy>,
                "ResourceManager::Add received an unsupported resource type.");

  if (!resource)
    return {};

  // 先读取名称再转移所有权；具体派生类是资源描述的唯一来源，
  // 注册路径不再临时改写 ID 或类别。
  const std::string assetId = resource->Id;
  const auto type = resource->Type;
  if (!MatchesType<StoredTy>(type))
    return {};
  return Add(std::move(assetId), std::move(resource));
}

template <typename T>
ResourceRef<StoredResourceTy<T>>
ResourceManager::Add(std::string assetId, std::unique_ptr<T> resource) {
  using StoredTy = StoredResourceTy<T>;
  static_assert(!std::is_void_v<StoredTy>,
                "ResourceManager::Add received an unsupported resource type.");

  // 所有 Mesh 无论来自内建构造还是文件导入，都在进入池前执行相同安全检查；
  // PreserveAuthored 用于阻止分裂法线或解析曲面法线被错误地再次平均。
  if constexpr (std::is_same_v<StoredTy, BaseMesh>) {
    if (!resource || !resource->Validate())
      return {};
    if (resource->NormalMode == NormalTy::Generate)
      resource->ComputeNormals();
  } else if constexpr (std::is_same_v<StoredTy, BaseTexture>) {
    if (!resource || !resource->Validate())
      return {};
  }
  return Pool<StoredTy>().Add(std::move(assetId), std::move(resource));
}

template <typename T> bool ResourceManager::MatchesType(ResourceTy type) {
  if constexpr (std::is_same_v<T, BaseMesh>)
    return type == ResourceTy::Mesh;
  if constexpr (std::is_same_v<T, BaseMaterial>)
    return type == ResourceTy::Material;
  if constexpr (std::is_same_v<T, BaseShaderComponent>)
    return type == ResourceTy::ShaderComponent;
  if constexpr (std::is_same_v<T, BaseShader>)
    return type == ResourceTy::Shader;
  if constexpr (std::is_same_v<T, BaseTexture>)
    return type == ResourceTy::Texture;
  return false;
}

template <typename T> ResourcePool<T> &ResourceManager::Pool() {
  // 类型到成员池的分派只在 const 版本维护一份；这里的 this 本身可写，因此恢复
  // 可写引用不会突破调用者的 const 边界。
  return const_cast<ResourcePool<T> &>(std::as_const(*this).Pool<T>());
}

template <typename T> const ResourcePool<T> &ResourceManager::Pool() const {
  if constexpr (std::is_same_v<T, BaseMesh>) {
    return Meshes;
  } else if constexpr (std::is_same_v<T, BaseMaterial>) {
    return Materials;
  } else if constexpr (std::is_same_v<T, BaseShaderComponent>) {
    return Shaders;
  } else if constexpr (std::is_same_v<T, BaseShader>) {
    return ShaderPrograms;
  } else if constexpr (std::is_same_v<T, BaseTexture>) {
    return Textures;
  } else {
    static_assert(
        !sizeof(T),
        "ResourceManager::Pool received an unsupported resource type.");
  }
}

template <typename ResourceTy>
ResourceRef<ResourceTy>
ResourceManager::Resolve(const ResourceRef<ResourceTy> &reference) const {
  // 合并后的引用允许调用方安全地重复解析；热路径已持有 Index 时不再查询字符串表。
  if (reference.IsResolved())
    return reference;
  return Pool<ResourceTy>().Find(reference.GetId());
}

template <typename ResourceTy>
ResourceTy *ResourceManager::TryGet(ResourceRef<ResourceTy> reference) {
  return Pool<ResourceTy>().TryGet(reference);
}

template <typename ResourceTy>
const ResourceTy *
ResourceManager::TryGet(ResourceRef<ResourceTy> reference) const {
  return Pool<ResourceTy>().TryGet(reference);
}

} // namespace z8
