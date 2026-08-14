#include "UI/Layout/ToggleNode.h"

#include "UI/Property/PropertyParser.h"
#include "UI/Style/Theme.h"
#include "Util/Color.h"

using namespace z8::ui;
using z8::EventReply;

ToggleNode::ToggleNode() {
  const auto &style = Theme::Default().Toggle;
  Focusable = true;
  Style.Direction = FlexDirection::Row;
  Style.Height = style.ControlHeight;
  Style.MinHeight = style.ControlHeight;
  Style.Padding = 0.0f;
  Style.FlexGrow = 0.0f;
  Style.FlexShrink = 0.0f;
  SetColor(Color::Transparent);
  SetBorder(Color::Transparent, 0.0f);

  auto indicator = std::make_unique<RectNode>();
  IndicatorNode = indicator.get();
  IndicatorNode->Key = "__indicator";
  IndicatorNode->HitTestVisible = false;
  IndicatorNode->Style.Width = style.IndicatorSize;
  IndicatorNode->Style.Height = style.IndicatorSize;
  IndicatorNode->Style.MinWidth = 0.0f;
  IndicatorNode->Style.MinHeight = 0.0f;
  IndicatorNode->Style.Margin =
      (style.ControlHeight - style.IndicatorSize) * 0.5f;
  IndicatorNode->Style.FlexGrow = 0.0f;
  IndicatorNode->Style.FlexShrink = 0.0f;
  IndicatorNode->SetCornerRadius(style.CornerRadius);
  BaseNode::AddChild(std::move(indicator));

  auto label = std::make_unique<TextNode>();
  LabelNode = label.get();
  LabelNode->Key = "__label";
  LabelNode->Style.Margin = style.ContentGap * 0.5f;
  LabelNode->Style.MinHeight = 0.0f;
  LabelNode->Style.FlexGrow = 1.0f;
  BaseNode::AddChild(std::move(label));
  OnVisualStateChanged();
}

bool ToggleNode::ConsumeChanged() {
  const bool result = ChangePending;
  ChangePending = false;
  return result;
}

EventReply ToggleNode::OnKeyDown(KeyArgs args) {
  if (!Enabled || (args.Key != VK_RETURN && args.Key != VK_SPACE))
    return EventReply::Ignored;
  if (!args.WasDown)
    SetChecked(!Checked);
  return EventReply::Handled;
}

EventReply ToggleNode::OnMouseDown(MouseMovArgs args) {
  if (!Enabled || args.Button != MouseButton::Left || !Contains(args))
    return EventReply::Ignored;
  Armed = true;
  return EventReply::Capture;
}

EventReply ToggleNode::OnMouseUp(MouseMovArgs args) {
  const bool toggles = Armed && Enabled && Contains(args);
  Armed = false;
  if (toggles)
    SetChecked(!Checked);
  return EventReply::Handled;
}

void ToggleNode::OnVisualStateChanged() {
  const auto &style = Theme::Default().Toggle;
  // Toggle 的根节点只负责整行命中；恢复通用样式后必须再次保持透明，实际
  // 反馈由 Indicator 和文字承担，避免 Immediate UI 复用时出现整块底色。
  SetColor(Color::Transparent);
  SetBorder(Color::Transparent, 0.0f);
  const auto interaction = !Enabled  ? WidgetVisualState::Disabled
                           : Pressed ? WidgetVisualState::Pressed
                           : Hovered ? WidgetVisualState::Hovered
                           : Focused ? WidgetVisualState::Focused
                                     : WidgetVisualState::Normal;
  const auto indicatorState =
      Checked && Enabled ? WidgetVisualState::Selected : interaction;
  if (IndicatorNode) {
    IndicatorNode->SetColor(style.IndicatorColor.Resolve(indicatorState));
    // Checked 使用实心 Accent，Focused 只使用 Accent 描边；二者不能共用
    // 填充色，否则取消勾选后焦点仍会被误读成持久选中。
    IndicatorNode->SetBorder(
        Focused && Enabled ? style.FocusedBorderColor : style.BorderColor,
        style.BorderWidth);
  }
  if (LabelNode)
    LabelNode->Color = style.ForegroundColor.Resolve(interaction);
}

bool ToggleNode::SetChecked(bool checked, bool notify) {
  if (Checked == checked)
    return false;
  Checked = checked;
  ChangePending = notify;
  OnVisualStateChanged();
  if (notify && ValueChanged)
    ValueChanged(Checked);
  return true;
}

void ToggleNode::SetEnabled(bool enabled) {
  Enabled = enabled;
  if (!Enabled)
    Armed = false;
  OnVisualStateChanged();
}

bool ToggleNode::SetProperty(const std::string &name,
                             const std::string &value) {
  if (name == "Text" || name == "Label") {
    SetText(value);
    return true;
  }
  if (name == "Checked" || name == "Value") {
    bool checked = false;
    return ParseBoolean(value, checked) && (SetChecked(checked, false), true);
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

void ToggleNode::SetText(const std::string &text) {
  if (LabelNode)
    LabelNode->Text = text;
}
