#include "UI/Layout/ScrollBarNode.h"

#include "UI/Style/Theme.h"

#include <algorithm>

using namespace z8::ui;
using z8::EventReply;

ScrollBarNode::ScrollBarNode(ScrollBarOrientation orientation)
    : ThumbNode(nullptr), Orientation(orientation) {
  const auto &style = Theme::Default().ScrollBar;
  SetColor(style.TrackColor);
  Style.Margin = 0.0f;
  Style.MinWidth = 0.0f;
  Style.MinHeight = 0.0f;

  auto thumb = std::make_unique<RectNode>();
  ThumbNode = thumb.get();
  ThumbNode->Key = "__thumb";
  ThumbNode->HitTestVisible = false;
  ThumbNode->Style.Position = PositionType::Absolute;
  ThumbNode->Style.Margin = 0.0f;
  ThumbNode->Style.MinWidth = 0.0f;
  ThumbNode->Style.MinHeight = 0.0f;
  ThumbNode->SetCornerRadius(style.CornerRadius);
  if (Orientation == ScrollBarOrientation::Vertical) {
    ThumbNode->Style.Left = style.ThumbInset;
    ThumbNode->Style.Right = style.ThumbInset;
  } else {
    ThumbNode->Style.Top = style.ThumbInset;
    ThumbNode->Style.Bottom = style.ThumbInset;
  }
  AddChild(std::move(thumb));
  OnVisualStateChanged();
}

void ScrollBarNode::OnVisualStateChanged() {
  if (!ThumbNode)
    return;
  const auto state = Pressed   ? WidgetVisualState::Pressed
                     : Hovered ? WidgetVisualState::Hovered
                               : WidgetVisualState::Normal;
  ThumbNode->SetColor(Theme::Default().ScrollBar.ThumbColor.Resolve(state));
}

void ScrollBarNode::SetMetrics(float viewportExtent, float contentExtent) {
  ViewportExtent = (std::max)(0.0f, viewportExtent);
  ContentExtent = (std::max)(ViewportExtent, contentExtent);
  Maximum = (std::max)(0.0f, ContentExtent - ViewportExtent);
  SetValue(Value, false);
}

void ScrollBarNode::SetValue(float value, bool notify) {
  const float next = std::clamp(value, 0.0f, Maximum);
  if (next == Value)
    return;
  Value = next;
  if (notify && ValueChanged)
    ValueChanged(Value);
}

EventReply ScrollBarNode::OnMouseDown(MouseMovArgs args) {
  if (args.Button != MouseButton::Left || !Visible)
    return EventReply::Ignored;
  if (ThumbNode->Contains(static_cast<float>(args.X),
                          static_cast<float>(args.Y))) {
    const float track =
        Orientation == ScrollBarOrientation::Vertical ? Height : Width;
    const float thumb = Orientation == ScrollBarOrientation::Vertical
                            ? ThumbNode->Height
                            : ThumbNode->Width;
    const float travel = (std::max)(0.0f, track - thumb);
    DragScale = travel > 0.0f ? Maximum / travel : 0.0f;
    DraggingThumb = true;
    return EventReply::Capture;
  }
  if (!Contains(static_cast<float>(args.X), static_cast<float>(args.Y)))
    return EventReply::Ignored;
  const float pointer = Orientation == ScrollBarOrientation::Vertical
                            ? static_cast<float>(args.Y)
                            : static_cast<float>(args.X);
  const float thumbStart = Orientation == ScrollBarOrientation::Vertical
                               ? ThumbNode->Top
                               : ThumbNode->Left;
  SetValue(Value + (pointer < thumbStart ? -ViewportExtent : ViewportExtent));
  // 轨道分页是瞬时操作，不应错误占用后续拖动事件。
  return EventReply::Handled;
}

EventReply ScrollBarNode::OnMouseDrag(MouseMovArgs args) {
  if (!DraggingThumb)
    return EventReply::Ignored;
  const float delta = Orientation == ScrollBarOrientation::Vertical
                          ? static_cast<float>(args.DeltaY)
                          : static_cast<float>(args.DeltaX);
  SetValue(Value + delta * DragScale);
  return EventReply::Handled;
}

EventReply ScrollBarNode::OnMouseUp(MouseMovArgs) {
  const bool handled = DraggingThumb;
  DraggingThumb = false;
  return handled ? EventReply::Handled : EventReply::Ignored;
}

void ScrollBarNode::OnAfterLayout() {
  const auto &style = Theme::Default().ScrollBar;
  const float track =
      Orientation == ScrollBarOrientation::Vertical ? Height : Width;
  // 最小滑块保证可操作性，但极小轨道仍必须夹紧，避免滑块越出控件裁剪区。
  const float thumb =
      ContentExtent > 0.0f
          ? (std::min)(track,
                       (std::max)(style.MinimumThumbLength,
                                  track * ViewportExtent / ContentExtent))
          : track;
  const float travel = (std::max)(0.0f, track - thumb);
  const float position = Maximum > 0.0f ? travel * Value / Maximum : 0.0f;
  if (Orientation == ScrollBarOrientation::Vertical) {
    ThumbNode->Style.Height = thumb;
    ThumbNode->Style.Top = position;
  } else {
    ThumbNode->Style.Width = thumb;
    ThumbNode->Style.Left = position;
  }
}
