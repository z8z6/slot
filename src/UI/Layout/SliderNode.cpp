#include "UI/Layout/SliderNode.h"

#include "UI/Property/PropertyParser.h"
#include "UI/Style/Theme.h"
#include "Util/Color.h"

#include <algorithm>
#include <cmath>

using namespace z8::ui;
using z8::EventReply;

SliderNode::SliderNode() {
  const auto &style = Theme::Default().Slider;
  Focusable = true;
  Style.Height = style.ControlHeight;
  Style.MinWidth = style.MinimumWidth;
  Style.MinHeight = style.ControlHeight;
  Style.Margin = SpacingStyle::ExtraSmall;
  Style.Padding = 0.0f;
  // Slider 在纵向表单中保持控件高度；横向可用空间由交叉轴拉伸提供，不能
  // 用 FlexGrow 消耗父容器的剩余高度。
  Style.FlexGrow = 0.0f;
  Style.FlexShrink = 0.0f;
  SetColor(Color::Transparent);
  SetBorder(Color::Transparent, 0.0f);

  auto track = std::make_unique<RectNode>();
  TrackNode = track.get();
  TrackNode->Key = "__track";
  TrackNode->HitTestVisible = false;
  TrackNode->Style.Position = PositionType::Absolute;
  TrackNode->Style.MinWidth = 0.0f;
  TrackNode->Style.MinHeight = 0.0f;
  TrackNode->Style.Margin = 0.0f;
  TrackNode->SetColor(style.TrackColor);
  TrackNode->SetCornerRadius(style.CornerRadius);
  BaseNode::AddChild(std::move(track));

  auto fill = std::make_unique<RectNode>();
  FillNode = fill.get();
  FillNode->Key = "__fill";
  FillNode->HitTestVisible = false;
  FillNode->Style.Position = PositionType::Absolute;
  FillNode->Style.MinWidth = 0.0f;
  FillNode->Style.MinHeight = 0.0f;
  FillNode->Style.Margin = 0.0f;
  FillNode->SetColor(style.FillColor);
  FillNode->SetCornerRadius(style.CornerRadius);
  BaseNode::AddChild(std::move(fill));

  auto thumb = std::make_unique<RectNode>();
  ThumbNode = thumb.get();
  ThumbNode->Key = "__thumb";
  ThumbNode->HitTestVisible = false;
  ThumbNode->Style.Position = PositionType::Absolute;
  ThumbNode->Style.MinWidth = 0.0f;
  ThumbNode->Style.MinHeight = 0.0f;
  ThumbNode->Style.Margin = 0.0f;
  ThumbNode->SetCornerRadius(style.ThumbSize * 0.5f);
  BaseNode::AddChild(std::move(thumb));
  OnVisualStateChanged();
}

bool SliderNode::ConsumeChanged() {
  const bool result = ChangePending;
  ChangePending = false;
  return result;
}

EventReply SliderNode::OnKeyDown(KeyArgs args) {
  if (!Enabled || (args.Key != VK_LEFT && args.Key != VK_RIGHT))
    return EventReply::Ignored;
  const float increment = Step > 0.0f ? Step : (Maximum - Minimum) / 100.0f;
  SetValue(Value + (args.Key == VK_LEFT ? -increment : increment));
  return EventReply::Handled;
}

EventReply SliderNode::OnMouseDown(MouseMovArgs args) {
  if (!Enabled || args.Button != MouseButton::Left || !Contains(args))
    return EventReply::Ignored;
  Dragging = true;
  UpdateFromPointer(static_cast<float>(args.X));
  return EventReply::Capture;
}

EventReply SliderNode::OnMouseDrag(MouseMovArgs args) {
  if (!Dragging)
    return EventReply::Ignored;
  UpdateFromPointer(static_cast<float>(args.X));
  return EventReply::Handled;
}

EventReply SliderNode::OnMouseUp(MouseMovArgs args) {
  if (!Dragging)
    return EventReply::Ignored;
  UpdateFromPointer(static_cast<float>(args.X));
  Dragging = false;
  return EventReply::Handled;
}

void SliderNode::OnVisualStateChanged() {
  const auto &style = Theme::Default().Slider;
  // Immediate UI 会在每帧恢复通用 Rect 样式，因此复合控件必须在状态同步时
  // 重建自己的透明命中面和轨道颜色，避免复用节点短暂显示普通矩形背景。
  SetColor(Color::Transparent);
  SetBorder(Color::Transparent, 0.0f);
  if (TrackNode)
    TrackNode->SetColor(style.TrackColor);
  if (FillNode)
    FillNode->SetColor(style.FillColor);
  if (!ThumbNode)
    return;
  const auto state = !Enabled  ? WidgetVisualState::Disabled
                     : Pressed ? WidgetVisualState::Pressed
                     : Hovered ? WidgetVisualState::Hovered
                     : Focused ? WidgetVisualState::Focused
                               : WidgetVisualState::Normal;
  ThumbNode->SetColor(style.ThumbColor.Resolve(state));
}

void SliderNode::SetEnabled(bool enabled) {
  Enabled = enabled;
  if (!Enabled)
    Dragging = false;
  OnVisualStateChanged();
}

bool SliderNode::SetProperty(const std::string &name,
                             const std::string &value) {
  if (name == "Minimum" || name == "Min") {
    float minimum = 0.0f;
    if (!ParseFiniteFloat(value, minimum))
      return false;
    Minimum = minimum;
    // XAML 属性无顺序语义；临时扩张另一端可让 Min/Max 任意顺序解析，最终
    // 仍保持非零范围，避免归一化时除零。
    if (Maximum <= Minimum)
      Maximum = Minimum + 1.0f;
    SetValue(Value, false);
    return true;
  }
  if (name == "Maximum" || name == "Max") {
    float maximum = 0.0f;
    if (!ParseFiniteFloat(value, maximum))
      return false;
    Maximum = maximum;
    if (Minimum >= Maximum)
      Minimum = Maximum - 1.0f;
    SetValue(Value, false);
    return true;
  }
  if (name == "Value") {
    float next = 0.0f;
    if (!ParseFiniteFloat(value, next))
      return false;
    SetValue(next, false);
    return true;
  }
  if (name == "Step") {
    float step = 0.0f;
    if (!ParseFiniteFloat(value, step) || step < 0.0f)
      return false;
    Step = step;
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

bool SliderNode::SetRange(float minimum, float maximum) {
  if (!std::isfinite(minimum) || !std::isfinite(maximum) || maximum <= minimum)
    return false;
  Minimum = minimum;
  Maximum = maximum;
  SetValue(Value, false);
  return true;
}

bool SliderNode::SetValue(float value, bool notify) {
  float next = std::clamp(value, Minimum, Maximum);
  if (Step > 0.0f)
    next = Minimum + std::round((next - Minimum) / Step) * Step;
  next = std::clamp(next, Minimum, Maximum);
  if (std::abs(next - Value) < 0.00001f)
    return false;
  Value = next;
  ChangePending = notify;
  if (notify && ValueChanged)
    ValueChanged(Value);
  return true;
}

void SliderNode::Synchronize() {
  RectNode::Synchronize();
  if (!TrackNode || !FillNode || !ThumbNode)
    return;
  const auto &style = Theme::Default().Slider;
  const float normalized =
      Maximum > Minimum ? (Value - Minimum) / (Maximum - Minimum) : 0.0f;
  const float trackTop = (Height - style.TrackThickness) * 0.5f;
  const float travel = (std::max)(0.0f, Width - style.ThumbSize);
  const float thumbLeft = travel * normalized;
  const float thumbTop = (Height - style.ThumbSize) * 0.5f;
  TrackNode->Computed = {0.0f, trackTop, Width, style.TrackThickness};
  FillNode->Computed = {0.0f, trackTop, thumbLeft + style.ThumbSize * 0.5f,
                        style.TrackThickness};
  ThumbNode->Computed = {thumbLeft, thumbTop, style.ThumbSize, style.ThumbSize};
}

void SliderNode::UpdateFromPointer(float clientX) {
  const float thumb = Theme::Default().Slider.ThumbSize;
  const float travel = (std::max)(1.0f, Width - thumb);
  const float normalized =
      std::clamp((clientX - Left - thumb * 0.5f) / travel, 0.0f, 1.0f);
  SetValue(Minimum + normalized * (Maximum - Minimum));
}
