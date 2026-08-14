#include "UI/Declarative/ImmediateUI.h"

#include "Object/UIObject/UIObject.h"
#include "UI/Declarative/ControlFactory.h"
#include "UI/Layout/BaseNode.h"
#include "UI/Layout/ButtonNode.h"
#include "UI/Layout/DrawNode.h"
#include "UI/Layout/ImageNode.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/MenuNode.h"
#include "UI/Layout/PanelGroupNode.h"
#include "UI/Layout/PanelNode.h"
#include "UI/Layout/SceneNode.h"
#include "UI/Layout/SliderNode.h"
#include "UI/Layout/TerminalNode.h"
#include "UI/Layout/TextInputNode.h"
#include "UI/Layout/ToggleNode.h"
#include "UI/Layout/ToolBarNode.h"
#include "UI/Style/Theme.h"

using namespace z8::ui;

ImmediateUI::ImmediateUI(Layout &layout) : TargetLayout(&layout) {}

void ImmediateUI::BeginFrame() {
  Error.clear();
  Changed = false;
  ScopeStack.clear();
  ScopeStack.push_back({TargetLayout->Root->ContentHost(), 0});
}

bool ImmediateUI::BeginMenu(const std::string &key, const std::string &text,
                            const UIStyle &style) {
  auto *menu = dynamic_cast<MenuNode *>(Acquire("Menu", key));
  if (!menu)
    return false;
  menu->SetText(text);
  ApplyStyle(*menu, style);
  ScopeStack.push_back({menu->ContentHost(), 0});
  return true;
}

bool ImmediateUI::Button(const std::string &key, const std::string &text,
                         const UIStyle &style) {
  auto *button = dynamic_cast<ButtonNode *>(Acquire("Button", key));
  if (!button)
    return false;
  button->SetText(text);
  ApplyStyle(*button, style);
  return button->ConsumeClicked();
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

void ImmediateUI::ResetStyle(BaseNode &node, bool keepsInteractiveGeometry) {
  const auto *panel = dynamic_cast<PanelNode *>(&node);
  const bool isScene = dynamic_cast<SceneNode *>(&node) != nullptr;
  const bool isImage = dynamic_cast<ImageNode *>(&node) != nullptr;
  const bool isButton = dynamic_cast<ButtonNode *>(&node) != nullptr;
  const bool isToggle = dynamic_cast<ToggleNode *>(&node) != nullptr;
  const bool isSlider = dynamic_cast<SliderNode *>(&node) != nullptr;
  const bool isTextInput = dynamic_cast<TextInputNode *>(&node) != nullptr;
  auto *menu = dynamic_cast<MenuNode *>(&node);
  const bool isMenuItem = dynamic_cast<MenuItemNode *>(&node) != nullptr;
  const bool isToolBar = dynamic_cast<ToolBarNode *>(&node) != nullptr;
  const RectStyle &style =
      panel ? static_cast<const RectStyle &>(Theme::Default().Panel)
            : Theme::Default().Rect;

  // Immediate UI 会复用稳定 key 对应的节点，因此省略字段表示恢复控件默认值，
  // 而不是继承上一帧的声明。交互后的窗口几何是运行时状态，需单独保留。
  if (!keepsInteractiveGeometry) {
    const float iconSize = Theme::Default().Icon.NormalSize;
    node.Style.Width = isImage ? std::optional<float>(iconSize) : std::nullopt;
    node.Style.Height = isImage ? std::optional<float>(iconSize) : std::nullopt;
    node.Style.Margin = isScene || isImage ? 0.0f : style.Margin;
  }
  const auto &sceneStyle = Theme::Default().Panel;
  node.Style.MinWidth = isImage   ? 0.0f
                        : isScene ? sceneStyle.MinWidth
                                  : style.MinWidth;
  node.Style.MinHeight = isImage   ? 0.0f
                         : isScene ? sceneStyle.MinHeight
                                   : style.MinHeight;
  node.Style.FlexGrow = isImage ? 0.0f : 1.0f;
  node.Style.FlexShrink = isImage ? 0.0f : 1.0f;
  node.Style.Padding = isScene || isImage ? 0.0f : style.Padding;
  node.Style.Direction = FlexDirection::Column;
  // 原子控件的盒模型是其交互契约的一部分；ImmediateUI 只重置声明覆盖，
  // 不能把 Button/Slider 退化成会纵向拉伸的普通 Rect。
  if (isButton) {
    const auto &button = Theme::Default().Button;
    node.Style.Height = button.ControlHeight;
    node.Style.MinHeight = button.ControlHeight;
    node.Style.FlexGrow = 0.0f;
    node.Style.FlexShrink = 0.0f;
    node.Style.Padding = button.ContentPadding;
    node.Style.Direction = FlexDirection::Row;
  } else if (isToggle) {
    const auto &toggle = Theme::Default().Toggle;
    node.Style.Height = toggle.ControlHeight;
    node.Style.MinHeight = toggle.ControlHeight;
    node.Style.FlexGrow = 0.0f;
    node.Style.FlexShrink = 0.0f;
    node.Style.Padding = 0.0f;
    node.Style.Direction = FlexDirection::Row;
  } else if (isSlider) {
    const auto &slider = Theme::Default().Slider;
    node.Style.Height = slider.ControlHeight;
    node.Style.MinWidth = slider.MinimumWidth;
    node.Style.MinHeight = slider.ControlHeight;
    node.Style.FlexGrow = 0.0f;
    node.Style.FlexShrink = 0.0f;
    node.Style.Padding = 0.0f;
  } else if (isTextInput) {
    const auto &input = Theme::Default().TextInput;
    node.Style.Height = input.ControlHeight;
    node.Style.MinWidth = input.MinimumWidth;
    node.Style.MinHeight = input.ControlHeight;
    node.Style.FlexGrow = 0.0f;
    node.Style.FlexShrink = 0.0f;
    node.Style.Padding = input.ContentPadding;
    node.Style.Direction = FlexDirection::Row;
  } else if (menu || isMenuItem) {
    const auto &menuStyle = Theme::Default().Menu;
    node.Style.Height = menu && menu->IsTopLevel() ? menuStyle.MenuBarHeight
                                                  : menuStyle.ItemHeight;
    node.Style.MinWidth = 0.0f;
    node.Style.MinHeight = *node.Style.Height;
    node.Style.FlexGrow = 0.0f;
    node.Style.FlexShrink = 0.0f;
    node.Style.Margin = 0.0f;
    node.Style.Padding = menuStyle.HorizontalPadding;
    node.Style.Direction = FlexDirection::Row;
    // SetTopLevel 同步恢复 Menu 自己的触发宽度、Chevron 与 Popup 锚点。
    if (menu)
      menu->SetTopLevel(menu->IsTopLevel());
  } else if (isToolBar) {
    const auto &toolbar = Theme::Default().ToolBar;
    node.Style.Height = toolbar.Height;
    node.Style.MinWidth = 0.0f;
    node.Style.MinHeight = toolbar.Height;
    node.Style.FlexGrow = 0.0f;
    node.Style.FlexShrink = 0.0f;
    node.Style.Margin = 0.0f;
    node.Style.Padding = toolbar.Padding;
    node.Style.Direction = FlexDirection::Row;
  }
  if (auto *visual = dynamic_cast<DrawNode *>(&node)) {
    // Image 的默认颜色是图标前景而非控件底色；否则省略 Tint 时图标会
    // 与深色 Rect 背景混为一体。图标也不继承容器边框和圆角。
    visual->SetColor(
        isImage ? Theme::Default().Icon.Color.Resolve(WidgetVisualState::Normal)
                : style.Color);
    visual->SetBorder(isImage ? DirectX::XMFLOAT4{0, 0, 0, 0}
                              : style.BorderColor,
                      isImage ? 0.0f : style.BorderWidth);
    visual->SetCornerRadius(isImage ? 0.0f : style.CornerRadius);
    if (isToolBar) {
      const auto &toolbar = Theme::Default().ToolBar;
      visual->SetColor(toolbar.Color);
      visual->SetBorder(toolbar.BorderColor, toolbar.BorderWidth);
      visual->SetCornerRadius(0.0f);
    }
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
  // 状态控件在 ResetStyle 后先恢复主题视觉，再应用本帧显式 Color/Border 覆盖。
  if (auto *buttonNode = dynamic_cast<ButtonNode *>(&node))
    buttonNode->SetEnabled(buttonNode->Enabled);
  else if (auto *toggleNode = dynamic_cast<ToggleNode *>(&node))
    toggleNode->SetEnabled(toggleNode->Enabled);
  else if (auto *sliderNode = dynamic_cast<SliderNode *>(&node))
    sliderNode->SetEnabled(sliderNode->Enabled);
  else if (auto *inputNode = dynamic_cast<TextInputNode *>(&node))
    inputNode->SetEnabled(inputNode->Enabled);
  else if (auto *menuNode = dynamic_cast<MenuNode *>(&node))
    menuNode->SetEnabled(menuNode->Enabled);
  else if (auto *menuItemNode = dynamic_cast<MenuItemNode *>(&node))
    menuItemNode->SetEnabled(menuItemNode->Enabled);
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
    const auto color =
        style.BorderColor ? *style.BorderColor : visual->UO->GetBorderColor();
    const float width =
        style.BorderWidth ? *style.BorderWidth : visual->UO->GetBorderWidth();
    visual->SetBorder(color, width);
    if (style.CornerRadius)
      visual->SetCornerRadius(*style.CornerRadius);
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
  ApplyStyle(*group, style);
  ScopeStack.push_back({panel->ContentHost(), 0});
  return true;
}

bool ImmediateUI::BeginToolBar(const std::string &key,
                               const UIStyle &style) {
  auto *toolbar = dynamic_cast<ToolBarNode *>(Acquire("ToolBar", key));
  if (!toolbar)
    return false;
  ApplyStyle(*toolbar, style);
  ScopeStack.push_back({toolbar->ContentHost(), 0});
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
                       ? dynamic_cast<TerminalNode *>(
                             group->AddPanel(std::make_unique<TerminalNode>()))
                       : dynamic_cast<TerminalNode *>(group->Panels.front());
  if (!terminal)
    return nullptr;
  terminal->SetProperty("Title", title);
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

void ImmediateUI::EndMenu() {
  if (ScopeStack.size() <= 1) {
    Error = "EndMenu has no matching BeginMenu.";
    return;
  }
  CloseScope();
}

void ImmediateUI::EndToolBar() {
  if (ScopeStack.size() <= 1) {
    Error = "EndToolBar has no matching BeginToolBar.";
    return;
  }
  CloseScope();
}

BaseNode *ImmediateUI::Image(const std::string &key, const std::string &source,
                             const UIStyle &style) {
  auto *node = Acquire("Image", key);
  auto *image = dynamic_cast<ImageNode *>(node);
  if (!image || !image->SetProperty("Source", source)) {
    Error = "Unsupported image source: " + source;
    return nullptr;
  }
  ApplyStyle(*image, style);
  return image;
}

bool ImmediateUI::MenuItem(const std::string &key, const std::string &text,
                           const UIStyle &style) {
  auto *item = dynamic_cast<MenuItemNode *>(Acquire("MenuItem", key));
  if (!item)
    return false;
  item->SetText(text);
  ApplyStyle(*item, style);
  return item->ConsumeClicked();
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

bool ImmediateUI::Slider(const std::string &key, float &value, float minimum,
                         float maximum, const UIStyle &style) {
  auto *slider = dynamic_cast<SliderNode *>(Acquire("Slider", key));
  if (!slider || !slider->SetRange(minimum, maximum))
    return false;
  const bool changed = slider->ConsumeChanged();
  if (changed)
    value = slider->Value;
  else
    slider->SetValue(value, false);
  ApplyStyle(*slider, style);
  return changed;
}

bool ImmediateUI::TextInput(const std::string &key, std::string &value,
                            const std::string &placeholder,
                            const UIStyle &style) {
  auto *input = dynamic_cast<TextInputNode *>(Acquire("TextInput", key));
  if (!input)
    return false;
  const bool changed = input->ConsumeChanged();
  if (changed)
    value = input->Text;
  else
    input->SetText(value, false);
  input->SetPlaceholder(placeholder);
  ApplyStyle(*input, style);
  return changed;
}

bool ImmediateUI::Toggle(const std::string &key, const std::string &text,
                         bool &value, const UIStyle &style) {
  auto *toggle = dynamic_cast<ToggleNode *>(Acquire("Toggle", key));
  if (!toggle)
    return false;
  const bool changed = toggle->ConsumeChanged();
  if (changed)
    value = toggle->Checked;
  else
    toggle->SetChecked(value, false);
  toggle->SetText(text);
  ApplyStyle(*toggle, style);
  return changed;
}

bool ImmediateUI::EndFrame() {
  if (ScopeStack.empty()) {
    Error = "BeginFrame must be called before EndFrame.";
    return false;
  }
  if (ScopeStack.size() != 1) {
    Error = "One or more control scopes were not closed.";
    while (ScopeStack.size() > 1)
      CloseScope();
  }
  CloseScope();
  if (Changed)
    TargetLayout->RebuildIndex();
  return Error.empty();
}
