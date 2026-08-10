#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace z8::ui {
class BaseNode;

/**
 * XAML 与即时声明共享的控件工厂。
 * 显式注册代替在解析器中硬编码分支，后续新增控件无需修改 XML 语法层。
 */
class ControlFactory {
public:
  using Creator = std::function<std::unique_ptr<BaseNode>()>;

  ControlFactory();
  void Register(const std::string& type, Creator creator);
  std::unique_ptr<BaseNode> Create(const std::string& type) const;

  static ControlFactory& Instance();

private:
  std::unordered_map<std::string, Creator> Creators;
};
} // namespace z8::ui
