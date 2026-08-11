#pragma once

#include <optional>
#include <string>
#include <vector>

#include <DirectXMath.h>

#include "yoga/YGEnums.h"

namespace z8::ui {
class BaseNode;
class Layout;

/** 常用 Yoga 样式的强类型声明；未填写字段不会覆盖控件已有样式。 */
struct UIStyle {
  std::optional<float> Width;
  std::optional<float> Height;
  std::optional<float> MinWidth;
  std::optional<float> MinHeight;
  std::optional<float> FlexGrow;
  std::optional<float> FlexShrink;
  std::optional<float> Margin;
  std::optional<float> Padding;
  std::optional<DirectX::XMFLOAT4> Color;
  std::optional<YGFlexDirection> Direction;
};

/**
 * ImGui 风格声明入口，但内部保留并复用控件。
 * 同级 key 和调用顺序稳定时不会重新分配 Yoga 节点或 UIObject。
 */
class ImmediateUI {
public:
  explicit ImmediateUI(Layout& layout);

  void BeginFrame();
  bool BeginPanel(const std::string& key, const std::string& title,
                  const UIStyle& style = {});
  void EndPanel();
  BaseNode* Rect(const std::string& key, const UIStyle& style = {});
  bool EndFrame();

  const std::string& LastError() const { return Error; }

private:
  struct Scope { BaseNode* Host; size_t NextChild; };
  Layout* TargetLayout;
  std::vector<Scope> ScopeStack;
  bool Changed = false;
  std::string Error;

  BaseNode* Acquire(const std::string& type, const std::string& key);
  static void ApplyStyle(BaseNode& node, const UIStyle& style);
  void CloseScope();
};
} // namespace z8::ui
