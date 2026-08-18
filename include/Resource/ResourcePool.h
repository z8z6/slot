#pragma once

#include "ResourceHandle.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace z8 {

/**
 * @brief ResourceManager 内部使用的分类型所有权容器。
 *
 * 每个槽位独占一个资源对象；外部只保存 Handle。当前阶段资源在应用退出前常驻，
 * 但 Generation 已为后续热重载和卸载保留安全边界。
 */
template <typename ResourceTy>
class ResourcePool {
public:
  /** 接管资源所有权并返回新句柄；空资源、空 ID 或重复 ID 返回无效句柄。 */
  ResourceHandle<ResourceTy> Add(std::string assetId,
                                 std::unique_ptr<ResourceTy> resource) {
    if (!resource || assetId.empty() || AssetIndex.find(assetId) != AssetIndex.end())
      return {};

    const auto index = static_cast<uint32_t>(Slots.size());
    Slots.push_back({std::move(resource), 1});
    const ResourceHandle<ResourceTy> handle{index, Slots.back().Generation};
    AssetIndex.emplace(std::move(assetId), handle);
    return handle;
  }

  /** 只解析已驻留资源，不触发加载；未知 ID 返回无效句柄。 */
  ResourceHandle<ResourceTy> Find(const std::string& assetId) const {
    const auto iterator = AssetIndex.find(assetId);
    return iterator == AssetIndex.end() ? ResourceHandle<ResourceTy>{}
                                        : iterator->second;
  }

  /** 校验 Index/Generation 后返回观察指针，旧句柄或越界句柄返回空。 */
  ResourceTy* TryGet(ResourceHandle<ResourceTy> handle) {
    return const_cast<ResourceTy*>(std::as_const(*this).TryGet(handle));
  }

  /** const 查询与可写查询保持相同的 Generation 有效性规则。 */
  const ResourceTy* TryGet(ResourceHandle<ResourceTy> handle) const {
    if (!handle.IsValid() || handle.Index >= Slots.size()) return nullptr;
    const auto& slot = Slots[handle.Index];
    if (slot.Generation != handle.Generation) return nullptr;
    return slot.Resource.get();
  }

  size_t Size() const { return AssetIndex.size(); }

  /** 按稳定槽位顺序访问驻留资源，供 GPU 缓存确定性构建。 */
  template <typename VisitorTy>
  void Visit(VisitorTy&& visitor) const {
    for (uint32_t index = 0; index < Slots.size(); ++index) {
      const auto& slot = Slots[index];
      if (slot.Resource)
        visitor(ResourceHandle<ResourceTy>{index, slot.Generation},
                *slot.Resource);
    }
  }

private:
  struct Slot {
    std::unique_ptr<ResourceTy> Resource;
    uint32_t Generation;
  };

  std::vector<Slot> Slots;
  std::unordered_map<std::string, ResourceHandle<ResourceTy>> AssetIndex;
};

} // namespace z8
