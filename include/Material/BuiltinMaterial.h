#pragma once

#include "Material.h"
#include "Resource/BuiltinResource.h"

namespace z8 {

/** 内建金属材质集中声明默认参数和稳定资源身份。 */
struct MetalMaterial : Material {
  MetalMaterial();
  std::string GetName() const override {
    return std::string(builtin::MetalMaterial);
  }
};

/** UI 材质独立成类型，避免注册处手工拼装名称和 Program 依赖。 */
struct UIMaterial : Material {
  UIMaterial();
  std::string GetName() const override {
    return std::string(builtin::UIMaterial);
  }
};

} // namespace z8
