#include "UI/Declarative/ImmediateUI.h"

#include "UI/Declarative/ControlFactory.h"
#include "UI/Layout/BaseNode.h"
#include "UI/Layout/DrawNode.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/PanelNode.h"
#include "yoga/YGNodeStyle.h"

using namespace z8::ui;

ImmediateUI::ImmediateUI(Layout &layout) : TargetLayout(&layout) {}

void ImmediateUI::BeginFrame() {
  Error.clear();
  Changed = false;
  ScopeStack.clear();
  ScopeStack.push_back({TargetLayout->Root->ContentHost(), 0});
}

BaseNode *ImmediateUI::Acquire(const std::string &type,
                               const std::string &key) {
  if (ScopeStack.empty()) {
    Error = "BeginFrame must be called before declaring controls.";
    return nullptr;
  }
  auto &scope = ScopeStack.back();
  if (key.empty()) {
    Error = "Immediate-mode controls require a non-empty key.";
    return nullptr;
  }
  for (size_t i = 0; i < scope.NextChild; ++i) {
    if (scope.Host->Children[i]->Key == key) {
      Error = "Duplicate key in the current scope: " + key;
      return nullptr;
    }
  }
  BaseNode *node = scope.NextChild < scope.Host->Children.size()
                       ? scope.Host->Children[scope.NextChild].get()
                       : nullptr;
  if (!node || node->Key != key || node->TypeName() != type) {
    // 顺序或类型变化后只截断受影响的后缀，稳定前缀仍保持对象和 Yoga 缓存。
    scope.Host->RemoveChildrenFrom(scope.NextChild);
    auto created = ControlFactory::Instance().Create(type);
    if (!created) {
      Error = "Unregistered control type: " + type;
      return nullptr;
    }
    created->Key = key;
    node = scope.Host->AddChild(std::move(created));
    Changed = true;
  }
  ++scope.NextChild;
  return node;
}

void ImmediateUI::ApplyStyle(BaseNode &node, const UIStyle &style) {
  auto *panel = dynamic_cast<PanelNode *>(&node);
  // Immediate UI 会逐帧重放初始样式；交互后的 Panel 几何必须优先，否则用户
  // 刚完成的拖动或缩放会在下一次声明时被 Width/Height 覆盖。
  const bool keepsInteractiveGeometry =
      panel && panel->HasInteractiveGeometry();
  if (style.Width && !keepsInteractiveGeometry)
    YGNodeStyleSetWidth(node.Node, *style.Width);
  if (style.Height && !keepsInteractiveGeometry)
    YGNodeStyleSetHeight(node.Node, *style.Height);
  if (style.MinWidth)
    YGNodeStyleSetMinWidth(node.Node, *style.MinWidth);
  if (style.MinHeight)
    YGNodeStyleSetMinHeight(node.Node, *style.MinHeight);
  if (style.FlexGrow)
    YGNodeStyleSetFlexGrow(node.Node, *style.FlexGrow);
  if (style.FlexShrink)
    YGNodeStyleSetFlexShrink(node.Node, *style.FlexShrink);
  if (style.Margin && !keepsInteractiveGeometry)
    YGNodeStyleSetMargin(node.Node, YGEdgeAll, *style.Margin);
  if (style.Padding)
    YGNodeStyleSetPadding(node.Node, YGEdgeAll, *style.Padding);
  // 强类型即时样式直接访问视觉层；纯布局节点没有颜色状态，也无需空对象占位。
  if (style.Color)
    if (auto *visual = dynamic_cast<DrawNode *>(&node))
      visual->SetColor(*style.Color);
  if (style.Direction)
    YGNodeStyleSetFlexDirection(node.Node, *style.Direction);
}

bool ImmediateUI::BeginPanel(const std::string &key, const std::string &title,
                             const UIStyle &style) {
  auto *node = Acquire("Panel", key);
  auto *panel = dynamic_cast<PanelNode *>(node);
  if (!panel)
    return false;
  panel->SetProperty("Title", title);
  ApplyStyle(*panel, style);
  ScopeStack.push_back({panel->ContentHost(), 0});
  return true;
}

void ImmediateUI::CloseScope() {
  if (ScopeStack.empty())
    return;
  auto scope = ScopeStack.back();
  if (scope.Host->Children.size() > scope.NextChild) {
    scope.Host->RemoveChildrenFrom(scope.NextChild);
    Changed = true;
  }
  ScopeStack.pop_back();
}

void ImmediateUI::EndPanel() {
  if (ScopeStack.size() <= 1) {
    Error = "EndPanel has no matching BeginPanel.";
    return;
  }
  CloseScope();
}

BaseNode *ImmediateUI::Rect(const std::string &key, const UIStyle &style) {
  auto *node = Acquire("Rect", key);
  if (node)
    ApplyStyle(*node, style);
  return node;
}

bool ImmediateUI::EndFrame() {
  if (ScopeStack.empty()) {
    Error = "BeginFrame must be called before EndFrame.";
    return false;
  }
  if (ScopeStack.size() != 1) {
    Error = "One or more panels were not closed.";
    while (ScopeStack.size() > 1)
      CloseScope();
  }
  CloseScope();
  if (Changed)
    TargetLayout->RebuildIndex();
  return Error.empty();
}
