#pragma once

#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <string_view>

namespace z8 {

/**
 * @brief 资源在序列化数据中的强类型软引用。
 *
 * AssetId 只在加载、场景序列化和编辑器边界使用；渲染热路径必须先将其解析为
 * ResourceHandle，避免逐帧执行字符串查询。
 */
template <typename ResourceTy>
class ResourceReference {
public:
  ResourceReference() = default;
  explicit ResourceReference(std::string_view assetId) : AssetId(assetId) {}

  bool IsValid() const { return !AssetId.empty(); }
  const std::string& GetAssetId() const { return AssetId; }

private:
  std::string AssetId;
};

/**
 * @brief 资源池中的强类型运行时句柄。
 *
 * Generation 用于识别资源槽位被回收后遗留的旧句柄；
 * 利用模板标识不同资源类型的句柄不能互换，从而在编译期阻止把 Material 当作 Mesh 使用。
 */
template <typename ResourceTy>
struct ResourceHandle {
  static constexpr uint32_t InvalidIndex = std::numeric_limits<uint32_t>::max();

  // 索引表示在相应对象池中的索引
  uint32_t Index = InvalidIndex;
  uint32_t Generation = 0;

  bool IsValid() const { return Index != InvalidIndex; }
  auto operator<=>(const ResourceHandle&) const = default;
};

/** 为 ResourceHandle 提供哈希，使后端缓存可以直接用强类型句柄作为键。 */
template <typename ResourceTy>
struct ResourceHandleHash {
  size_t operator()(const ResourceHandle<ResourceTy>& handle) const noexcept {
    const size_t index = std::hash<uint32_t>{}(handle.Index);
    const size_t generation = std::hash<uint32_t>{}(handle.Generation);
    return index ^ (generation << 1U);
  }
};

} // namespace z8
