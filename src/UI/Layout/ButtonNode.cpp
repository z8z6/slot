#include "UI/Layout/ButtonNode.h"

#include "UI/Property/PropertyParser.h"
#include "UI/Style/Theme.h"

using namespace z8::ui;
using z8::EventReply;

ButtonNode::ButtonNode() {
  const auto &style = Theme::Default().Button;
  Focusable = true;
  Style.Direction = FlexDirection::Row;
  Style.Height = style.ControlHeight;
  Style.MinHeight = style.ControlHeight;
  Style.Padding = style.ContentPadding;
  Style.FlexGrow = 0.0f;
  Style.FlexShrink = 0.0f;
  SetBorder(style.BorderColor, style.BorderWidth);
  SetCornerRadius(style.CornerRadius);

  auto label = std::make_unique<TextNode>();
  LabelNode = label.get();
  LabelNode->Key = "__label";
  LabelNode->Alignment = TextAlignment::Center;
  LabelNode->Style.Margin = 0.0f;
  LabelNode->Style.MinHeight = 0.0f;
  LabelNode->Style.FlexGrow = 1.0f;
  BaseNode::AddChild(std::move(label));
  OnVisualStateChanged();
}

void ButtonNode::Activate() {
  if (!Enabled)
    return;
  ClickPending = true;
  if (Clicked)
    Clicked();
}

bool ButtonNode::ConsumeClicked() {
  const bool result = ClickPending;
  ClickPending = false;
  return result;
}

EventReply ButtonNode::OnKeyDown(KeyArgs args) {
  if (!Enabled || (args.Key != VK_RETURN && args.Key != VK_SPACE))
    return EventReply::Ignored;
  if (!args.WasDown)
    Activate();
  return EventReply::Handled;
}

EventReply ButtonNode::OnMouseDown(MouseMovArgs args) {
  if (!Enabled || args.Button != MouseButton::Left || !Contains(args))
    return EventReply::Ignored;
  Armed = true;
  return EventReply::Capture;
}

EventReply ButtonNode::OnMouseUp(MouseMovArgs args) {
  const bool activates = Armed && Enabled && Contains(args);
  Armed = false;
  if (activates)
    Activate();
  return EventReply::Handled;
}

void ButtonNode::OnVisualStateChanged() {
  const auto &style = Theme::Default().Button;
  const auto state = !Enabled  ? WidgetVisualState::Disabled
                     : Pressed ? WidgetVisualState::Pressed
                     : Hovered ? WidgetVisualState::Hovered
                     : Focused ? WidgetVisualState::Focused
                               : WidgetVisualState::Normal;
  SetColor(style.BackgroundColor.Resolve(state));
  if (LabelNode)
    LabelNode->Color = style.ForegroundColor.Resolve(state);
}

bool ButtonNode::SetProperty(const std::string &name,
                             const std::string &value) {
  if (name == "Text" || name == "Label") {
    SetText(value);
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

void ButtonNode::SetEnabled(bool enabled) {
  Enabled = enabled;
  if (!Enabled)
    Armed = false;
  OnVisualStateChanged();
}

void ButtonNode::SetText(const std::string &text) {
  if (LabelNode)
    LabelNode->Text = text;
}
