#pragma once

#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <string_view>

namespace z8 {
enum class ResourceTy {
  None,
  Mesh,
  Material,
  ShaderComponent,
  Shader,
  Texture,
  Audio,
};

class Resource {
public:
  std::string Id;
  ResourceTy Type = ResourceTy::None;

  virtual ~Resource() = default;
  [[nodiscard]] bool IsValid() const { return !Id.empty(); }
};

/**
 * @brief 资源的强类型引用，同时覆盖持久化 ID 与运行时索引两种边界。
 *
 * 场景和材质使用 Asset ID 构造待解析引用；ResourceManager::Resolve 将其转换为
 * 只携带稳定池索引的已解析引用，供渲染热路径和 GPU 缓存使用。资源池当前只追加且
 * 不复用槽位，因此索引在 Manager 生命周期内稳定，不需要额外的版本字段。
 */
template <typename ResourceTy> class ResourceRef {
private:
  static constexpr uint32_t Invalid = std::numeric_limits<uint32_t>::max();
  std::string Id;

public:
  uint32_t Index = Invalid;

  /** 构造既没有 Asset ID 也没有池索引的空引用。 */
  ResourceRef() = default;
  /** 构造供资源访问和缓存使用的已解析引用。 */
  explicit ResourceRef(uint32_t index) : Index(index) {}
  /** 构造供场景、材质和序列化边界保存的待解析引用。 */
  explicit ResourceRef(std::string_view id) : Id(id) {}

  /** 返回待解析 Asset ID；已解析引用不继续携带字符串。 */
  [[nodiscard]] const std::string &GetId() const { return Id; }
  /** 只有获得池索引后，引用才可用于资源访问和后端缓存。 */
  [[nodiscard]] bool IsResolved() const { return Index != Invalid; }
  /** Asset ID 或池索引任一存在时，引用都具有可继续处理的有效状态。 */
  [[nodiscard]] bool IsValid() const { return IsResolved() || !Id.empty(); }

  /** 同时比较引用状态与载荷，避免不同待解析 ID 被视为同一引用。 */
  bool operator==(const ResourceRef &other) const {
    return Index == other.Index && Id == other.Id;
  }

  /** 提供与相等运算一致的否定比较。 */
  bool operator!=(const ResourceRef &other) const { return !(*this == other); }
};

/** 为 ResourceRef 提供哈希，使已解析引用可直接作为后端缓存键。 */
template <typename ResourceTy> struct ResourceRefHash {
  size_t operator()(const ResourceRef<ResourceTy> &reference) const noexcept {
    const size_t index = std::hash<uint32_t>{}(reference.Index);
    const size_t id = std::hash<std::string>{}(reference.GetId());
    return index ^ (id << 1U);
  }
};

} // namespace z8
