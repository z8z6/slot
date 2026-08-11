#include "UI/Layout/ScrollBarNode.h"

#include "UI/Style/UITheme.h"
#include "yoga/YGNodeStyle.h"

#include <algorithm>

using namespace z8::ui;

ScrollBarNode::ScrollBarNode(UIScrollBarOrientation orientation)
    : ThumbNode(nullptr), Orientation(orientation) {
  const auto& style = UITheme::Modern().Panel;
  SetColor(style.ScrollBarColor);
  YGNodeStyleSetMargin(GetYogaNode(), YGEdgeAll, 0.0f);
  YGNodeStyleSetMinWidth(GetYogaNode(), 0.0f);
  YGNodeStyleSetMinHeight(GetYogaNode(), 0.0f);

  auto thumb = std::make_unique<RectNode>();
  ThumbNode = thumb.get();
  ThumbNode->Key = "__thumb";
  ThumbNode->SetColor(style.ScrollThumbColor);
  YGNodeStyleSetPositionType(ThumbNode->GetYogaNode(), YGPositionTypeAbsolute);
  YGNodeStyleSetMargin(ThumbNode->GetYogaNode(), YGEdgeAll, 0.0f);
  YGNodeStyleSetMinWidth(ThumbNode->GetYogaNode(), 0.0f);
  YGNodeStyleSetMinHeight(ThumbNode->GetYogaNode(), 0.0f);
  if (Orientation == UIScrollBarOrientation::Vertical) {
    YGNodeStyleSetPosition(ThumbNode->GetYogaNode(), YGEdgeLeft, 2.0f);
    YGNodeStyleSetPosition(ThumbNode->GetYogaNode(), YGEdgeRight, 2.0f);
  } else {
    YGNodeStyleSetPosition(ThumbNode->GetYogaNode(), YGEdgeTop, 2.0f);
    YGNodeStyleSetPosition(ThumbNode->GetYogaNode(), YGEdgeBottom, 2.0f);
  }
  AddChild(std::move(thumb));
}

void ScrollBarNode::SetMetrics(float viewportExtent, float contentExtent) {
  ViewportExtent = (std::max)(0.0f, viewportExtent);
  ContentExtent = (std::max)(ViewportExtent, contentExtent);
  Maximum = (std::max)(0.0f, ContentExtent - ViewportExtent);
  SetValue(Value, false);
}

void ScrollBarNode::SetValue(float value, bool notify) {
  const float next = std::clamp(value, 0.0f, Maximum);
  if (next == Value) return;
  Value = next;
  if (notify && ValueChanged) ValueChanged(Value);
}

bool ScrollBarNode::OnMouseDown(MouseMovArgs args) {
  if (args.Button != MouseButton::Left || !IsVisible()) return false;
  if (ThumbNode->Contains(static_cast<float>(args.X),
                          static_cast<float>(args.Y))) {
    const float track = Orientation == UIScrollBarOrientation::Vertical
        ? GetLayoutHeight() : GetLayoutWidth();
    const float thumb = Orientation == UIScrollBarOrientation::Vertical
        ? ThumbNode->GetLayoutHeight() : ThumbNode->GetLayoutWidth();
    const float travel = (std::max)(0.0f, track - thumb);
    DragScale = travel > 0.0f ? Maximum / travel : 0.0f;
    DraggingThumb = true;
    return true;
  }
  if (!Contains(static_cast<float>(args.X), static_cast<float>(args.Y)))
    return false;
  const float pointer = Orientation == UIScrollBarOrientation::Vertical
      ? static_cast<float>(args.Y) : static_cast<float>(args.X);
  const float thumbStart = Orientation == UIScrollBarOrientation::Vertical
      ? ThumbNode->GetLayoutY() : ThumbNode->GetLayoutX();
  SetValue(Value + (pointer < thumbStart ? -ViewportExtent : ViewportExtent));
  return true;
}

bool ScrollBarNode::OnMouseDrag(MouseMovArgs args) {
  if (!DraggingThumb) return false;
  const float delta = Orientation == UIScrollBarOrientation::Vertical
      ? static_cast<float>(args.DeltaY) : static_cast<float>(args.DeltaX);
  SetValue(Value + delta * DragScale);
  return true;
}

bool ScrollBarNode::OnMouseUp(MouseMovArgs) {
  const bool handled = DraggingThumb;
  DraggingThumb = false;
  return handled;
}

void ScrollBarNode::OnLayoutUpdated() {
  const auto& style = UITheme::Modern().Panel;
  const float track = Orientation == UIScrollBarOrientation::Vertical
      ? GetLayoutHeight() : GetLayoutWidth();
  // 最小滑块保证可操作性，但极小轨道仍必须夹紧，避免滑块越出控件裁剪区。
  const float thumb = ContentExtent > 0.0f
      ? (std::min)(track,
                   (std::max)(style.MinimumScrollThumbLength,
                              track * ViewportExtent / ContentExtent))
      : track;
  const float travel = (std::max)(0.0f, track - thumb);
  const float position = Maximum > 0.0f ? travel * Value / Maximum : 0.0f;
  if (Orientation == UIScrollBarOrientation::Vertical) {
    YGNodeStyleSetHeight(ThumbNode->GetYogaNode(), thumb);
    YGNodeStyleSetPosition(ThumbNode->GetYogaNode(), YGEdgeTop, position);
  } else {
    YGNodeStyleSetWidth(ThumbNode->GetYogaNode(), thumb);
    YGNodeStyleSetPosition(ThumbNode->GetYogaNode(), YGEdgeLeft, position);
  }
}
