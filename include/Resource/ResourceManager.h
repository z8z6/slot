#pragma once

#include "Material/Material.h"
#include "Mesh/Mesh.h"
#include "ResourcePool.h"
#include "Shader/Shader.h"
#include "Shader/ShaderProgram.h"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace z8 {

/** 把具体内建派生类归一到 ResourceManager 实际存储的资源基类。 */
template <typename T>
using StoredResourceTy = std::conditional_t<
    std::is_base_of_v<Mesh, T>, Mesh,
    std::conditional_t<std::is_base_of_v<Material, T>, Material,
                       std::conditional_t<std::is_same_v<Shader, T>, Shader,
                                          std::conditional_t<
                                              std::is_same_v<ShaderProgram, T>,
                                              ShaderProgram, void>>>>;

/**
 * @brief 应用级资源所有者和类型安全查询入口。
 *
 * Manager 本身不是单例：Application 显式拥有它，DX12 等后端只缓存对应的 GPU
 * 表示。这样一个进程可安全创建多个测试上下文，并为未来的多 Device 留出边界。
 */
class ResourceManager {
  ResourcePool<Mesh> Meshes;
  ResourcePool<Material> Materials;
  ResourcePool<Shader> Shaders;
  ResourcePool<ShaderProgram> ShaderPrograms;

public:
  /** 创建应用级资源上下文，并按确定顺序注册全部内建资源。 */
  ResourceManager();

  /**
   * 根据资源类自动选择类型池，并使用对象的 GetName() 作为稳定资源 ID。
   *
   * 该入口服务于内建类型和 manifest 生成资源；文件导入等序列化边界使用带
   * assetId 的 Add 重载。
   */
  template <typename T>
  ResourceHandle<StoredResourceTy<T>> Add(std::unique_ptr<T> resource);

  /**
   * 使用加载边界提供的 ID 接管资源；适用于 FBX 等名称不属于 builtin URI 的资源。
   */
  template <typename T>
  ResourceHandle<StoredResourceTy<T>>
  Add(std::string assetId, std::unique_ptr<T> resource);

  const ResourcePool<Material>& GetMaterials() const { return Materials; }
  const ResourcePool<Mesh>& GetMeshes() const { return Meshes; }
  const ResourcePool<ShaderProgram>& GetShaderPrograms() const {
    return ShaderPrograms;
  }
  const ResourcePool<Shader>& GetShaders() const { return Shaders; }

  /** 把持久化软引用解析为运行时句柄，不执行 I/O。 */
  template <typename ResourceTy>
  ResourceHandle<ResourceTy>
  Resolve(const ResourceRef<ResourceTy>& reference) const;

  /** 仅在句柄仍属于当前槽位 Generation 时返回资源观察指针。 */
  template <typename ResourceTy>
  ResourceTy* TryGet(ResourceHandle<ResourceTy> handle);

  /** const 版本用于后端只读缓存构建。 */
  template <typename ResourceTy>
  const ResourceTy* TryGet(ResourceHandle<ResourceTy> handle) const;

private:
  /** 根据规范名称类别校验自动注册的 C++ 资源类型。 */
  template <typename T>
  static bool MatchesType(ResourceTy type);

  /** 返回指定资源基类唯一对应的类型池，集中维护类型到存储的映射。 */
  template <typename T>
  ResourcePool<T>& Pool();

  /** const 池访问供 Resolve 和只读后端使用。 */
  template <typename T>
  const ResourcePool<T>& Pool() const;

  void RegisterBuiltinResources();
};

template <typename T>
ResourceHandle<StoredResourceTy<T>>
ResourceManager::Add(std::unique_ptr<T> resource) {
  using StoredTy = StoredResourceTy<T>;
  static_assert(!std::is_void_v<StoredTy>,
                "ResourceManager::Add received an unsupported resource type.");

  // 先读取名称再转移所有权；这样各资源类型只维护一份规范 ID，注册表不会再与类
  // 声明发生大小写或拼写漂移。
  const std::string assetId = resource ? resource->GetName() : std::string{};
  const auto type = ResourceId{assetId}.GetType();
  if (!MatchesType<StoredTy>(type)) return {};
  return Add(std::move(assetId), std::move(resource));
}

template <typename T>
ResourceHandle<StoredResourceTy<T>>
ResourceManager::Add(std::string assetId, std::unique_ptr<T> resource) {
  using StoredTy = StoredResourceTy<T>;
  static_assert(!std::is_void_v<StoredTy>,
                "ResourceManager::Add received an unsupported resource type.");

  // 所有 Mesh 无论来自内建构造还是文件导入，都在进入池前执行相同安全检查；
  // PreserveAuthored 用于阻止分裂法线或解析曲面法线被错误地再次平均。
  if constexpr (std::is_same_v<StoredTy, Mesh>) {
    if (!resource || !resource->Validate()) return {};
    if (resource->NormalMode == MeshNormalMode::GenerateSmooth)
      resource->ComputeNormals();
  }
  return Pool<StoredTy>().Add(std::move(assetId), std::move(resource));
}

template <typename T>
bool ResourceManager::MatchesType(ResourceTy type) {
  if constexpr (std::is_same_v<T, Mesh>) return type == ResourceTy::Mesh;
  if constexpr (std::is_same_v<T, Material>)
    return type == ResourceTy::Material;
  if constexpr (std::is_same_v<T, Shader>) return type == ResourceTy::Shader;
  if constexpr (std::is_same_v<T, ShaderProgram>)
    return type == ResourceTy::ShaderProgram;
  return false;
}

template <typename T>
ResourcePool<T>& ResourceManager::Pool() {
  // 类型到成员池的分派只在 const 版本维护一份；这里的 this 本身可写，因此恢复
  // 可写引用不会突破调用者的 const 边界。
  return const_cast<ResourcePool<T>&>(std::as_const(*this).Pool<T>());
}

template <typename T>
const ResourcePool<T>& ResourceManager::Pool() const {
  if constexpr (std::is_same_v<T, Mesh>) {
    return Meshes;
  } else if constexpr (std::is_same_v<T, Material>) {
    return Materials;
  } else if constexpr (std::is_same_v<T, Shader>) {
    return Shaders;
  } else if constexpr (std::is_same_v<T, ShaderProgram>) {
    return ShaderPrograms;
  } else {
    static_assert(!sizeof(T),
                  "ResourceManager::Pool received an unsupported resource type.");
  }
}

template <typename ResourceTy>
ResourceHandle<ResourceTy> ResourceManager::Resolve(
    const ResourceRef<ResourceTy>& reference) const {
  return Pool<ResourceTy>().Find(reference.GetId());
}

template <typename ResourceTy>
ResourceTy* ResourceManager::TryGet(ResourceHandle<ResourceTy> handle) {
  return Pool<ResourceTy>().TryGet(handle);
}

template <typename ResourceTy>
const ResourceTy*
ResourceManager::TryGet(ResourceHandle<ResourceTy> handle) const {
  return Pool<ResourceTy>().TryGet(handle);
}

} // namespace z8
