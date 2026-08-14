#include "UI/Layout/MenuNode.h"

#include "UI/Style/Theme.h"
#include "Util/Color.h"

#include <algorithm>

using namespace z8::ui;
using z8::EventReply;

namespace {

bool ParseBool(const std::string &value, bool &result) {
  if (value == "true" || value == "True" || value == "1")
    result = true;
  else if (value == "false" || value == "False" || value == "0")
    result = false;
  else
    return false;
  return true;
}

size_t Utf8CodePointCount(const std::string &text) {
  return static_cast<size_t>(std::count_if(
      text.begin(), text.end(), [](char value) {
        return (static_cast<unsigned char>(value) & 0xC0U) != 0x80U;
      }));
}

float TriggerWidth(const std::string &text) {
  const auto &theme = Theme::Default();
  // DirectWrite 度量位于渲染器；布局阶段用字体平均 advance 估算稳定宽度，
  // 与 Tab 的无设备回退路径一致，避免 Menu 构造依赖图形设备。
  return static_cast<float>(Utf8CodePointCount(text)) *
             theme.Text.FontSize * 0.56f +
         theme.Menu.HorizontalPadding * 3.0f + theme.Menu.IconSize;
}

} // namespace

MenuItemNode::MenuItemNode() {
  const auto &style = Theme::Default().Menu;
  Focusable = true;
  Style.Direction = FlexDirection::Row;
  Style.Height = style.ItemHeight;
  Style.MinWidth = 0.0f;
  Style.MinHeight = style.ItemHeight;
  Style.Margin = 0.0f;
  Style.Padding = style.HorizontalPadding;
  Style.FlexGrow = 0.0f;
  Style.FlexShrink = 0.0f;
  SetBorder(Color::Transparent, 0.0f);
  SetCornerRadius(style.CornerRadius);

  auto label = std::make_unique<TextNode>();
  LabelNode = label.get();
  LabelNode->Key = "__label";
  LabelNode->Style.Margin = 0.0f;
  LabelNode->Style.MinWidth = 0.0f;
  LabelNode->Style.MinHeight = 0.0f;
  LabelNode->Style.FlexGrow = 1.0f;
  BaseNode::AddChild(std::move(label));
  OnVisualStateChanged();
}

void MenuItemNode::Activate() {
  if (!Enabled)
    return;
  ClickPending = true;
  if (Clicked)
    Clicked();
  CloseOwningMenu();
}

void MenuItemNode::CloseOwningMenu() {
  for (auto *node = Parent; node; node = node->Parent) {
    auto *menu = dynamic_cast<MenuNode *>(node);
    if (!menu)
      continue;
    menu->CloseHierarchy();
    return;
  }
}

bool MenuItemNode::ConsumeClicked() {
  const bool result = ClickPending;
  ClickPending = false;
  return result;
}

EventReply MenuItemNode::OnKeyDown(KeyArgs args) {
  if (!Enabled || (args.Key != VK_RETURN && args.Key != VK_SPACE))
    return EventReply::Ignored;
  if (!args.WasDown)
    Activate();
  return EventReply::Handled;
}

EventReply MenuItemNode::OnMouseDown(MouseMovArgs args) {
  if (!Enabled || args.Button != MouseButton::Left || !Contains(args))
    return EventReply::Ignored;
  Armed = true;
  return EventReply::Capture;
}

EventReply MenuItemNode::OnMouseUp(MouseMovArgs args) {
  const bool activates = Armed && Enabled && Contains(args);
  Armed = false;
  if (activates)
    Activate();
  return EventReply::Handled;
}

void MenuItemNode::OnVisualStateChanged() {
  const auto &style = Theme::Default().Menu;
  const auto state = !Enabled  ? WidgetVisualState::Disabled
                     : Pressed ? WidgetVisualState::Pressed
                     : Hovered ? WidgetVisualState::Hovered
                     : Focused ? WidgetVisualState::Focused
                               : WidgetVisualState::Normal;
  SetColor(style.BackgroundColor.Resolve(state));
  if (LabelNode)
    LabelNode->Color = style.ForegroundColor.Resolve(state);
}

void MenuItemNode::SetEnabled(bool enabled) {
  Enabled = enabled;
  if (!Enabled)
    Armed = false;
  OnVisualStateChanged();
}

bool MenuItemNode::SetProperty(const std::string &name,
                               const std::string &value) {
  if (name == "Text" || name == "Label") {
    SetText(value);
    return true;
  }
  if (name == "Enabled") {
    bool enabled = false;
    if (!ParseBool(value, enabled))
      return false;
    SetEnabled(enabled);
    return true;
  }
  return RectNode::SetProperty(name, value);
}

void MenuItemNode::SetText(const std::string &text) {
  if (LabelNode)
    LabelNode->Text = text;
}

MenuNode::MenuNode() {
  const auto &style = Theme::Default().Menu;
  Focusable = true;
  Style.Direction = FlexDirection::Row;
  Style.Height = style.ItemHeight;
  Style.MinWidth = 0.0f;
  Style.MinHeight = style.ItemHeight;
  Style.Margin = 0.0f;
  Style.Padding = style.HorizontalPadding;
  Style.FlexGrow = 0.0f;
  Style.FlexShrink = 0.0f;
  SetBorder(Color::Transparent, 0.0f);
  SetCornerRadius(style.CornerRadius);

  auto label = std::make_unique<TextNode>();
  LabelNode = label.get();
  LabelNode->Key = "__label";
  LabelNode->Style.Margin = 0.0f;
  LabelNode->Style.MinWidth = 0.0f;
  LabelNode->Style.MinHeight = 0.0f;
  LabelNode->Style.FlexGrow = 1.0f;
  BaseNode::AddChild(std::move(label));

  auto chevron = std::make_unique<ImageNode>();
  ChevronNode = chevron.get();
  ChevronNode->Key = "__chevron";
  ChevronNode->HitTestVisible = false;
  ChevronNode->Style.Width = style.IconSize;
  ChevronNode->Style.Height = style.IconSize;
  ChevronNode->Style.MinWidth = 0.0f;
  ChevronNode->Style.MinHeight = 0.0f;
  ChevronNode->Style.Margin = 0.0f;
  ChevronNode->Style.FlexGrow = 0.0f;
  ChevronNode->Style.FlexShrink = 0.0f;
  BaseNode::AddChild(std::move(chevron));

  auto popup = std::make_unique<RectNode>();
  PopupNode = popup.get();
  PopupNode->Key = "__popup";
  PopupNode->Style.Position = PositionType::Absolute;
  PopupNode->Style.Direction = FlexDirection::Column;
  PopupNode->Style.Width = style.PopupWidth;
  PopupNode->Style.Left = style.PopupWidth - style.BorderWidth;
  PopupNode->Style.Top = 0.0f;
  PopupNode->Style.MinWidth = style.PopupWidth;
  PopupNode->Style.MinHeight = 0.0f;
  PopupNode->Style.Margin = 0.0f;
  PopupNode->Style.Padding = style.PopupPadding;
  PopupNode->Style.FlexGrow = 0.0f;
  PopupNode->Style.FlexShrink = 0.0f;
  PopupNode->SetColor(style.PopupColor);
  PopupNode->SetBorder(style.PopupBorderColor, style.BorderWidth);
  PopupNode->SetCornerRadius(style.CornerRadius);
  // DirectWrite 在 DX12 几何之后统一提交；声明通用遮挡语义后，Layout 会把
  // 更早的 Panel 文字裁出 Popup 区域，而渲染后端无需识别 Menu 类型。
  PopupNode->OccludesEarlierText = true;
  PopupNode->Visible = false;
  BaseNode::AddChild(std::move(popup));
  RefreshGeometry();
  OnVisualStateChanged();
}

void MenuNode::CloseBranch() { SetOpen(false); }

void MenuNode::CloseHierarchy() { RootMenu()->SetOpen(false); }

void MenuNode::CloseSiblingMenus() {
  BaseNode *host = ParentMenu() ? ParentMenu()->PopupNode : Parent;
  if (!host)
    return;
  for (const auto &child : host->Children) {
    auto *menu = dynamic_cast<MenuNode *>(child.get());
    if (menu && menu != this)
      menu->CloseBranch();
  }
}

bool MenuNode::HasOpenSibling() const {
  const auto *parentMenu = ParentMenu();
  const BaseNode *host = parentMenu ? parentMenu->PopupNode : Parent;
  if (!host)
    return false;
  for (const auto &child : host->Children) {
    const auto *menu = dynamic_cast<const MenuNode *>(child.get());
    if (menu && menu != this && menu->Open)
      return true;
  }
  return false;
}

EventReply MenuNode::OnKeyDown(KeyArgs args) {
  if (!Enabled)
    return EventReply::Ignored;
  if (args.Key == VK_ESCAPE) {
    CloseHierarchy();
    return EventReply::Handled;
  }
  if (args.Key == VK_LEFT && !TopLevel) {
    SetOpen(false);
    return EventReply::Handled;
  }
  if (args.Key == VK_RIGHT && !TopLevel) {
    OpenExclusive();
    return EventReply::Handled;
  }
  if (args.Key == VK_RETURN || args.Key == VK_SPACE) {
    if (!args.WasDown) {
      if (Open)
        SetOpen(false);
      else
        OpenExclusive();
    }
    return EventReply::Handled;
  }
  return EventReply::Ignored;
}

EventReply MenuNode::OnMouseDown(MouseMovArgs args) {
  if (!Enabled || args.Button != MouseButton::Left || !Contains(args))
    return EventReply::Ignored;
  if (Open)
    SetOpen(false);
  else
    OpenExclusive();
  return EventReply::Handled;
}

EventReply MenuNode::OnMouseMove(MouseMovArgs args) {
  if (!Enabled || !Contains(args))
    return EventReply::Ignored;
  // 级联目录悬停即展开；菜单栏只有在另一入口已展开时才切换，普通经过不会
  // 意外弹出菜单。
  if (!TopLevel || HasOpenSibling())
    OpenExclusive();
  return EventReply::Handled;
}

void MenuNode::OnAfterLayout() {
  if (!PopupNode)
    return;
  const auto &style = Theme::Default().Menu;
  const bool overflowsRight =
      Left + Width + style.PopupWidth > VisibleClip.z;
  // 级联目录优先向右展开；接近窗口右边界时翻到当前条目左侧，避免深层菜单
  // 被客户区裁掉。顶层菜单仍贴齐触发项，并在右侧空间不足时向左对齐。
  PopupNode->Style.Left = TopLevel
                              ? (Left + style.PopupWidth > VisibleClip.z
                                     ? Width - style.PopupWidth
                                     : 0.0f)
                              : (overflowsRight
                                     ? -style.PopupWidth + style.BorderWidth
                                     : Width - style.BorderWidth);
  PopupNode->Style.Top = TopLevel ? Height - style.BorderWidth : 0.0f;
}

void MenuNode::OnVisualStateChanged() {
  const auto &style = Theme::Default().Menu;
  const auto state = !Enabled ? WidgetVisualState::Disabled
                     : Open   ? WidgetVisualState::Selected
                     : Pressed ? WidgetVisualState::Pressed
                     : Hovered ? WidgetVisualState::Hovered
                     : Focused ? WidgetVisualState::Focused
                               : WidgetVisualState::Normal;
  SetColor(style.BackgroundColor.Resolve(state));
  if (LabelNode)
    LabelNode->Color = style.ForegroundColor.Resolve(state);
  if (ChevronNode)
    ChevronNode->SetColor(style.ForegroundColor.Resolve(state));
}

void MenuNode::OpenExclusive() {
  if (!Enabled)
    return;
  CloseSiblingMenus();
  SetOpen(true);
}

MenuNode *MenuNode::ParentMenu() const {
  for (auto *node = Parent; node; node = node->Parent)
    if (auto *menu = dynamic_cast<MenuNode *>(node))
      return menu;
  return nullptr;
}

void MenuNode::RefreshGeometry() {
  const auto &style = Theme::Default().Menu;
  Style.Height = TopLevel ? style.MenuBarHeight : style.ItemHeight;
  Style.MinHeight = *Style.Height;
  Style.Width = TopLevel && LabelNode
                    ? std::optional<float>(TriggerWidth(LabelNode->Text))
                    : std::nullopt;
  if (ChevronNode)
    ChevronNode->SetIcon(TopLevel ? UIIcon::ChevronDown
                                  : UIIcon::ChevronRight);
  if (PopupNode) {
    PopupNode->Style.Left = TopLevel ? 0.0f
                                     : style.PopupWidth - style.BorderWidth;
    PopupNode->Style.Top = TopLevel ? style.MenuBarHeight - style.BorderWidth
                                    : 0.0f;
  }
}

MenuNode *MenuNode::RootMenu() {
  auto *root = this;
  while (auto *parent = root->ParentMenu())
    root = parent;
  return root;
}

void MenuNode::SetEnabled(bool enabled) {
  Enabled = enabled;
  if (!Enabled)
    SetOpen(false);
  OnVisualStateChanged();
}

bool MenuNode::SetOpen(bool open) {
  if (!Enabled)
    open = false;
  if (Open == open)
    return false;
  Open = open;
  if (PopupNode)
    PopupNode->Visible = Open;
  if (!Open && PopupNode) {
    for (const auto &child : PopupNode->Children)
      if (auto *menu = dynamic_cast<MenuNode *>(child.get()))
        menu->CloseBranch();
  }
  OnVisualStateChanged();
  return true;
}

bool MenuNode::SetProperty(const std::string &name,
                           const std::string &value) {
  if (name == "Text" || name == "Label") {
    SetText(value);
    return true;
  }
  if (name == "Open" || name == "Expanded") {
    bool open = false;
    return ParseBool(value, open) && (SetOpen(open), true);
  }
  if (name == "Enabled") {
    bool enabled = false;
    if (!ParseBool(value, enabled))
      return false;
    SetEnabled(enabled);
    return true;
  }
  return RectNode::SetProperty(name, value);
}

void MenuNode::SetText(const std::string &text) {
  if (LabelNode)
    LabelNode->Text = text;
  RefreshGeometry();
}

void MenuNode::SetTopLevel(bool topLevel) {
  TopLevel = topLevel;
  RefreshGeometry();
}
