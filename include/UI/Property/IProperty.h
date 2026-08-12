#pragma once

#include <string>

namespace z8::ui {

/**
 * 所有可声明 UI 属性能力的共同接口，供 XAML/检查器统一发现
 */
class IProperty {
public:
  virtual ~IProperty() = default;
  virtual bool SetProperty(const std::string& name, const std::string& value) {
    return false;
  }
};
} // namespace z8::ui
