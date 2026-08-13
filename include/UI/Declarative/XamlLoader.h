#pragma once

#include "UI/Layout/BaseNode.h"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

namespace z8::ui {
class ControlFactory;
class Layout;

struct XamlLoadResult {
  std::unique_ptr<BaseNode> Root;
  std::string Error;
  // 错误的位置
  size_t ErrorOffset = 0;

  // LoadInto 成功后 Root 会移交给 Layout，因此成功与否只由 Error 表示。
  explicit operator bool() const { return Error.empty(); }
};

/**
 * 轻量 XAML 子集加载器。
 * 它只解析控件树和属性，不引入 XML 运行库；不支持文本节点、命名空间和数据绑定。
 */
class XamlLoader {
public:
  explicit XamlLoader(ControlFactory& factory);
  XamlLoader();

  XamlLoadResult Load(std::string_view source) const;
  XamlLoadResult LoadInto(Layout& layout, std::string_view source) const;
  XamlLoadResult LoadFileInto(Layout& layout, const std::string& fileName) const;

private:
  ControlFactory* Factory;
};
} // namespace z8::ui
