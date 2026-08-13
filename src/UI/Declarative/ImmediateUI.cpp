#include "UI/Declarative/ImmediateUI.h"

#include "UI/Declarative/ControlFactory.h"
#include "Object/UIObject/UIObject.h"
#include "UI/Layout/BaseNode.h"
#include "UI/Layout/DrawNode.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/PanelNode.h"
#include "UI/Layout/PanelGroupNode.h"
#include "UI/Layout/SceneNode.h"
#include "UI/Layout/TerminalNode.h"
#include "UI/Style/Theme.h"

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
    // 顺序或类型变化后只截断受影响的后缀，稳定前缀仍保持对象和布局状态。
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

void ImmediateUI::ResetStyle(BaseNode &node,
                             bool keepsInteractiveGeometry) {
  const auto *panel = dynamic_cast<PanelNode *>(&node);
  const bool isScene = dynamic_cast<SceneNode *>(&node) != nullptr;
  const RectStyle &style = panel ? static_cast<const RectStyle &>(
                                      Theme::Default().Panel)
                                 : Theme::Default().Rect;

  // Immediate UI 会复用稳定 key 对应的节点，因此省略字段表示恢复控件默认值，
  // 而不是继承上一帧的声明。交互后的窗口几何是运行时状态，需单独保留。
  if (!keepsInteractiveGeometry) {
    node.Style.Width.reset();
    node.Style.Height.reset();
    node.Style.Margin = isScene ? 0.0f : style.Margin;
  }
  const auto &sceneStyle = Theme::Default().Panel;
  node.Style.MinWidth = isScene ? sceneStyle.MinWidth : style.MinWidth;
  node.Style.MinHeight = isScene ? sceneStyle.MinHeight : style.MinHeight;
  node.Style.FlexGrow = 1.0f;
  node.Style.FlexShrink = 1.0f;
  node.Style.Padding = isScene ? 0.0f : style.Padding;
  node.Style.Direction = FlexDirection::Column;
  if (auto *visual = dynamic_cast<DrawNode *>(&node)) {
    visual->SetColor(style.Color);
    visual->SetBorder(style.BorderColor, style.BorderWidth);
  }
}

void ImmediateUI::ApplyStyle(BaseNode &node, const UIStyle &style) {
  auto *panel = dynamic_cast<PanelNode *>(&node);
  auto *group = dynamic_cast<PanelGroupNode *>(&node);
  auto *scene = dynamic_cast<SceneNode *>(&node);
  // Immediate UI 会逐帧重放初始样式；交互后的 Panel 几何必须优先，否则用户
  // 刚完成的拖动或缩放会在下一次声明时被 Width/Height 覆盖。
  const bool keepsInteractiveGeometry =
      (panel && panel->HasInteractiveGeometry()) ||
      (group && group->HasInteractiveGeometry()) ||
      (scene && scene->HasInteractiveGeometry());
  ResetStyle(node, keepsInteractiveGeometry);
  if (style.Width && !keepsInteractiveGeometry)
    node.Style.Width = *style.Width;
  if (style.Height && !keepsInteractiveGeometry)
    node.Style.Height = *style.Height;
  if (style.MinWidth)
    node.Style.MinWidth = *style.MinWidth;
  if (style.MinHeight)
    node.Style.MinHeight = *style.MinHeight;
  if (style.FlexGrow)
    node.Style.FlexGrow = *style.FlexGrow;
  if (style.FlexShrink)
    node.Style.FlexShrink = *style.FlexShrink;
  if (style.Margin && !keepsInteractiveGeometry)
    node.Style.Margin = *style.Margin;
  if (style.Padding)
    node.Style.Padding = *style.Padding;
  // 强类型即时样式直接访问视觉层；纯布局节点没有颜色状态，也无需空对象占位。
  if (style.Color)
    if (auto *visual = dynamic_cast<DrawNode *>(&node))
      visual->SetColor(*style.Color);
  if (auto *visual = dynamic_cast<DrawNode *>(&node)) {
    const auto color = style.BorderColor
                           ? *style.BorderColor
                           : visual->UO->GetBorderColor();
    const float width = style.BorderWidth
                            ? *style.BorderWidth
                            : visual->UO->GetBorderWidth();
    visual->SetBorder(color, width);
  }
  if (style.Direction)
    node.Style.Direction = *style.Direction;
}

bool ImmediateUI::BeginPanel(const std::string &key, const std::string &title,
                             const UIStyle &style) {
  auto *node = Acquire("PanelGroup", key);
  auto *group = dynamic_cast<PanelGroupNode *>(node);
  if (!group)
    return false;
  auto *panel = group->Panels.empty()
                    ? group->AddPanel(std::make_unique<PanelNode>())
                    : group->Panels.front();
  if (!panel)
    return false;
  panel->SetProperty("Title", title);
  if (!group->Tabs.empty())
    group->Tabs.front()->LabelNode->Text = title;
  ApplyStyle(*group, style);
  ScopeStack.push_back({panel->ContentHost(), 0});
  return true;
}

BaseNode *ImmediateUI::Terminal(const std::string &key,
                                const std::string &title,
                                const UIStyle &style) {
  auto *node = Acquire("PanelGroup", key);
  auto *group = dynamic_cast<PanelGroupNode *>(node);
  if (!group)
    return nullptr;
  auto *terminal = group->Panels.empty()
                       ? dynamic_cast<TerminalNode *>(group->AddPanel(
                             std::make_unique<TerminalNode>()))
                       : dynamic_cast<TerminalNode *>(group->Panels.front());
  if (!terminal)
    return nullptr;
  terminal->SetProperty("Title", title);
  if (!group->Tabs.empty())
    group->Tabs.front()->LabelNode->Text = title;
  ApplyStyle(*group, style);
  return terminal;
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

BaseNode *ImmediateUI::Scene(const std::string &key, const UIStyle &style) {
  auto *node = Acquire("Scene", key);
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
