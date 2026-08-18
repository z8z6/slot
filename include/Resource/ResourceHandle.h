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
  Shader,
  ShaderProgram,
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
 * @brief 资源在序列化数据中的强类型软引用。
 * 使用时必须先将其解析为 ResourceHandle
 */
template <typename ResourceTy>
class ResourceRef {
  std::string Id;

public:
  ResourceRef() = default;
  explicit ResourceRef(std::string_view id) : Id(id) {}

  [[nodiscard]] const std::string &GetId() const { return Id; }
};

/**
 * @brief 资源池中的强类型运行时句柄
 * Generation 用于识别资源槽位被回收后遗留的旧句柄；
 * 利用模板标识不同资源类型的句柄不能互换，从而在编译期阻止把 Material 当作 Mesh
 * 使用。
 */
template <typename ResourceTy>
class ResourceHandle {
public:
  uint32_t Index = Invalid;
  uint32_t Generation = 0;

  [[nodiscard]]
  bool IsValid() const {
    return Index != Invalid;
  }

  bool operator==(const ResourceHandle &other) const {
    return Index == other.Index && Generation == other.Generation;
  }

  bool operator!=(const ResourceHandle &other) const {
    return !(*this == other);
  }

private:
  static constexpr uint32_t Invalid = std::numeric_limits<uint32_t>::max();
};

template <typename ResourceTy> struct ResourceHandleHash {
  size_t operator()(const ResourceHandle<ResourceTy> &handle) const noexcept {
    const size_t index = std::hash<uint32_t>{}(handle.Index);
    const size_t generation = std::hash<uint32_t>{}(handle.Generation);
    return index ^ (generation << 1U);
  }
};

} // namespace z8
