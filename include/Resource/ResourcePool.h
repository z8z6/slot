#pragma once

#include "ResourceRef.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace z8 {

/**
 * @brief ResourceManager 内部使用的分类型所有权容器。
 *
 * 每个槽位独占一个资源对象；外部只保存已解析的 ResourceRef。资源在应用退出前
 * 常驻且槽位不复用，因此 Index 在 ResourceManager 生命周期内保持稳定。
 */
template <typename ResourceTy>
class ResourcePool {
private:
  std::vector<std::unique_ptr<ResourceTy>> Slots;
  std::unordered_map<std::string, ResourceRef<ResourceTy>> AssetIndex;

public:
  /** 接管资源所有权并返回已解析引用；空资源、空 ID 或重复 ID 返回空引用。 */
  ResourceRef<ResourceTy> Add(std::string assetId,
                              std::unique_ptr<ResourceTy> resource) {
    if (!resource || assetId.empty() ||
        AssetIndex.find(assetId) != AssetIndex.end())
      return {};

    const auto index = static_cast<uint32_t>(Slots.size());
    Slots.push_back(std::move(resource));
    const ResourceRef<ResourceTy> reference(index);
    AssetIndex.emplace(std::move(assetId), reference);
    return reference;
  }

  /** 只解析已驻留资源，不触发加载；未知 ID 返回空引用。 */
  ResourceRef<ResourceTy> Find(const std::string &assetId) const {
    const auto iterator = AssetIndex.find(assetId);
    return iterator == AssetIndex.end() ? ResourceRef<ResourceTy>{}
                                        : iterator->second;
  }

  size_t Size() const { return AssetIndex.size(); }

  /** 校验已解析 Index 后返回观察指针，待解析或越界引用返回空。 */
  ResourceTy *TryGet(ResourceRef<ResourceTy> reference) {
    return const_cast<ResourceTy *>(std::as_const(*this).TryGet(reference));
  }

  /** const 查询与可写查询保持相同的索引有效性规则。 */
  const ResourceTy *TryGet(ResourceRef<ResourceTy> reference) const {
    if (!reference.IsResolved() || reference.Index >= Slots.size())
      return nullptr;
    return Slots[reference.Index].get();
  }

  /** 按稳定槽位顺序访问驻留资源，供 GPU 缓存确定性构建。 */
  template <typename VisitorTy>
  void Visit(VisitorTy &&visitor) const {
    for (uint32_t index = 0; index < Slots.size(); ++index) {
      const auto &resource = Slots[index];
      if (resource)
        visitor(ResourceRef<ResourceTy>(index), *resource);
    }
  }
};

} // namespace z8
