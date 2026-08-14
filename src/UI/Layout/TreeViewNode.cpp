#include "UI/Layout/TreeViewNode.h"

#include "UI/Property/PropertyParser.h"
#include "UI/Style/Theme.h"
#include "Util/Color.h"

using namespace z8::ui;
using z8::EventReply;

TreeViewItemNode::TreeViewItemNode() {
  const auto &style = Theme::Default().TreeView;
  Focusable = true;
  Style.Direction = FlexDirection::Column;
  Style.Margin = 0.0f;
  Style.Padding = 0.0f;
  Style.MinHeight = style.RowHeight;
  Style.FlexGrow = 0.0f;
  Style.FlexShrink = 0.0f;
  SetColor(Color::Transparent);
  SetBorder(Color::Transparent, 0.0f);

  auto header = std::make_unique<RectNode>();
  HeaderNode = header.get();
  HeaderNode->Key = "__header";
  HeaderNode->HitTestVisible = false;
  HeaderNode->Style.Direction = FlexDirection::Row;
  HeaderNode->Style.Height = style.RowHeight;
  HeaderNode->Style.MinHeight = style.RowHeight;
  HeaderNode->Style.Margin = 0.0f;
  HeaderNode->Style.Padding = 0.0f;
  HeaderNode->Style.FlexGrow = 0.0f;
  HeaderNode->Style.FlexShrink = 0.0f;

  auto indent = std::make_unique<BaseNode>();
  IndentNode = indent.get();
  IndentNode->Key = "__indent";
  IndentNode->Style.Width = 0.0f;
  IndentNode->Style.MinWidth = 0.0f;
  IndentNode->Style.Margin = 0.0f;
  IndentNode->Style.FlexGrow = 0.0f;
  IndentNode->Style.FlexShrink = 0.0f;
  HeaderNode->BaseNode::AddChild(std::move(indent));

  auto expander = std::make_unique<ImageNode>();
  ExpanderNode = expander.get();
  ExpanderNode->Key = "__expander";
  ExpanderNode->SetIcon(UIIcon::ChevronDown);
  ExpanderNode->HitTestVisible = false;
  ExpanderNode->Style.Width = style.IconSize;
  ExpanderNode->Style.Height = style.IconSize;
  ExpanderNode->Style.Margin = (style.RowHeight - style.IconSize) * 0.5f;
  HeaderNode->BaseNode::AddChild(std::move(expander));

  auto label = std::make_unique<TextNode>();
  LabelNode = label.get();
  LabelNode->Key = "__label";
  LabelNode->Style.Margin = style.ContentGap * 0.5f;
  LabelNode->Style.MinHeight = 0.0f;
  LabelNode->Style.FlexGrow = 1.0f;
  HeaderNode->BaseNode::AddChild(std::move(label));
  BaseNode::AddChild(std::move(header));

  auto items = std::make_unique<BaseNode>();
  ItemsNode = items.get();
  ItemsNode->Key = "__items";
  ItemsNode->Style.Direction = FlexDirection::Column;
  ItemsNode->Style.Margin = 0.0f;
  ItemsNode->Style.Padding = 0.0f;
  ItemsNode->Style.FlexGrow = 0.0f;
  ItemsNode->Style.FlexShrink = 0.0f;
  BaseNode::AddChild(std::move(items));
  OnVisualStateChanged();
}

void TreeViewItemNode::Attach(TreeViewNode *owner, size_t depth) {
  Owner = owner;
  Depth = depth;
  if (IndentNode)
    IndentNode->Style.Width =
        static_cast<float>(Depth) * Theme::Default().TreeView.Indent;
  RefreshExpander();
}

bool TreeViewItemNode::HasItems() const {
  return ItemsNode && !ItemsNode->Children.empty();
}

EventReply TreeViewItemNode::OnKeyDown(KeyArgs args) {
  if (!Enabled)
    return EventReply::Ignored;
  if (args.Key == VK_LEFT && Expanded && HasItems()) {
    SetExpanded(false);
    return EventReply::Handled;
  }
  if (args.Key == VK_RIGHT && !Expanded && HasItems()) {
    SetExpanded(true);
    return EventReply::Handled;
  }
  if (args.Key == VK_RETURN || args.Key == VK_SPACE) {
    if (!args.WasDown && Owner)
      Owner->SelectItem(this);
    return EventReply::Handled;
  }
  return EventReply::Ignored;
}

EventReply TreeViewItemNode::OnMouseDown(MouseMovArgs args) {
  if (!Enabled || args.Button != MouseButton::Left || !HeaderNode ||
      !HeaderNode->Contains(args))
    return EventReply::Ignored;
  if (ExpanderNode && ExpanderNode->Visible && ExpanderNode->Contains(args))
    SetExpanded(!Expanded);
  else if (Owner)
    Owner->SelectItem(this);
  else
    SetSelected(true);
  return EventReply::Handled;
}

void TreeViewItemNode::OnVisualStateChanged() {
  if (!HeaderNode)
    return;
  const auto &style = Theme::Default().TreeView;
  const auto state = !Enabled   ? WidgetVisualState::Disabled
                     : Pressed  ? WidgetVisualState::Pressed
                     : Hovered  ? WidgetVisualState::Hovered
                     : Selected ? WidgetVisualState::Selected
                     : Focused  ? WidgetVisualState::Focused
                                : WidgetVisualState::Normal;
  HeaderNode->SetColor(style.RowColor.Resolve(state));
  if (LabelNode)
    LabelNode->Color = style.ForegroundColor.Resolve(state);
  if (ExpanderNode)
    ExpanderNode->SetColor(style.IconColor.Resolve(state));
}

void TreeViewItemNode::RefreshExpander() {
  if (!ExpanderNode || !ItemsNode)
    return;
  ExpanderNode->Visible = HasItems();
  if (HasItems())
    ExpanderNode->SetIcon(Expanded ? UIIcon::ChevronDown
                                   : UIIcon::ChevronRight);
  ItemsNode->Visible = Expanded;
  // LayoutEngine 当前仍度量不可见节点，因此折叠时用显式 0 高度同步移除流占位。
  ItemsNode->Style.Height =
      Expanded ? std::nullopt : std::optional<float>(0.0f);
}

void TreeViewItemNode::SetEnabled(bool enabled) {
  Enabled = enabled;
  OnVisualStateChanged();
}

bool TreeViewItemNode::SetExpanded(bool expanded) {
  if (Expanded == expanded)
    return false;
  Expanded = expanded;
  RefreshExpander();
  return true;
}

bool TreeViewItemNode::SetProperty(const std::string &name,
                                   const std::string &value) {
  if (name == "Text" || name == "Label") {
    SetText(value);
    return true;
  }
  if (name == "Expanded") {
    bool expanded = false;
    return ParseBoolean(value, expanded) && (SetExpanded(expanded), true);
  }
  if (name == "Selected") {
    bool selected = false;
    if (!ParseBoolean(value, selected))
      return false;
    SetSelected(selected);
    return true;
  }
  if (name == "Enabled") {
    bool enabled = false;
    if (!ParseBoolean(value, enabled))
      return false;
    SetEnabled(enabled);
    return true;
  }
  return RectNode::SetProperty(name, value);
}

void TreeViewItemNode::SetSelected(bool selected) {
  Selected = selected;
  OnVisualStateChanged();
}

void TreeViewItemNode::SetText(const std::string &text) {
  if (LabelNode)
    LabelNode->Text = text;
}

TreeViewNode::TreeViewNode() {
  ContentNode->Style.Padding = 0.0f;
  ContentNode->Style.Direction = FlexDirection::Column;
  Style.MinWidth = Theme::Default().Rect.MinWidth;
  Style.MinHeight = Theme::Default().TreeView.RowHeight;
}

void TreeViewNode::AttachItems(BaseNode &host, size_t depth,
                               TreeViewItemNode *&declaredSelection) {
  for (const auto &child : host.Children) {
    auto *item = dynamic_cast<TreeViewItemNode *>(child.get());
    if (!item)
      continue;
    item->Attach(this, depth);
    // XAML 可声明初始 Selected。首次布局选择第一个声明项并清除重复项；每帧
    // 重扫还能在动态删除子树后撤销 SelectedItem 的悬空观察指针。
    if (item->Selected) {
      if (!declaredSelection)
        declaredSelection = item;
      else
        item->SetSelected(false);
    }
    AttachItems(*item->ItemsNode, depth + 1, declaredSelection);
  }
}

void TreeViewNode::ClearSelection(BaseNode &host, TreeViewItemNode *except) {
  for (const auto &child : host.Children) {
    auto *item = dynamic_cast<TreeViewItemNode *>(child.get());
    if (!item)
      continue;
    item->SetSelected(item == except);
    ClearSelection(*item->ItemsNode, except);
  }
}

void TreeViewNode::OnAfterLayout() {
  TreeViewItemNode *declaredSelection = nullptr;
  AttachItems(*ContentNode, 0, declaredSelection);
  SelectedItem = declaredSelection;
}

bool TreeViewNode::SelectItem(TreeViewItemNode *item, bool notify) {
  if (item == SelectedItem)
    return false;
  SelectedItem = item;
  ClearSelection(*ContentNode, item);
  if (notify && SelectionChanged)
    SelectionChanged(SelectedItem);
  return true;
}
