#include "UI/Behavior/ResizeBehavior.h"

#include "UI/Layout/BaseNode.h"
#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"

#include <algorithm>
#include <cstdlib>

using namespace z8::ui;

namespace {

bool ParseBoolean(const std::string &value, bool &result) {
  if (value == "true" || value == "True" || value == "1") {
    result = true;
    return true;
  }
  if (value == "false" || value == "False" || value == "0") {
    result = false;
    return true;
  }
  return false;
}

} // namespace

void ResizeBehavior::SetProperties(const ResizeProperty &properties) {
  Properties = properties;
  Properties.Border = (std::max)(0.0f, Properties.Border);
  Properties.MinWidth = (std::max)(1.0f, Properties.MinWidth);
  Properties.MinHeight = (std::max)(1.0f, Properties.MinHeight);
  ApplyMinimumSize();
}

void ResizeBehavior::OnAttached() { ApplyMinimumSize(); }

void ResizeBehavior::ApplyMinimumSize() const {
  if (!GetOwner())
    return;
  YGNodeStyleSetMinWidth(GetOwner()->GetYogaNode(), Properties.MinWidth);
  YGNodeStyleSetMinHeight(GetOwner()->GetYogaNode(), Properties.MinHeight);
}

ResizeRegion ResizeBehavior::HitTest(MouseMovArgs args) const {
  const auto *owner = GetOwner();
  if (!owner || !Properties.Enabled || !owner->Contains(args))
    return ResizeRegion::None;
  const float x = static_cast<float>(args.X);
  const float y = static_cast<float>(args.Y);
  const bool left = x - owner->GetLayoutX() <= Properties.Border;
  const bool right =
      owner->GetLayoutX() + owner->GetLayoutWidth() - x <= Properties.Border;
  const bool top = y - owner->GetLayoutY() <= Properties.Border;
  const bool bottom =
      owner->GetLayoutY() + owner->GetLayoutHeight() - y <= Properties.Border;

  if (top && left)
    return ResizeRegion::TopLeft;
  if (top && right)
    return ResizeRegion::TopRight;
  if (bottom && left)
    return ResizeRegion::BottomLeft;
  if (bottom && right)
    return ResizeRegion::BottomRight;
  if (left)
    return ResizeRegion::Left;
  if (right)
    return ResizeRegion::Right;
  if (top)
    return ResizeRegion::Top;
  if (bottom)
    return ResizeRegion::Bottom;
  return ResizeRegion::None;
}

z8::MouseCursor ResizeBehavior::CursorForRegion(ResizeRegion region) {
  switch (region) {
  case ResizeRegion::Left:
  case ResizeRegion::Right:
    return MouseCursor::SizeHorizontal;
  case ResizeRegion::Top:
  case ResizeRegion::Bottom:
    return MouseCursor::SizeVertical;
  case ResizeRegion::TopLeft:
  case ResizeRegion::BottomRight:
    return MouseCursor::SizeDiagonalNorthwestSoutheast;
  case ResizeRegion::TopRight:
  case ResizeRegion::BottomLeft:
    return MouseCursor::SizeDiagonalNortheastSouthwest;
  default:
    return MouseCursor::Arrow;
  }
}

z8::MouseCursor ResizeBehavior::GetMouseCursor(MouseMovArgs args) const {
  // 捕获期间保持起始边对应的光标，鼠标越界后仍能表达当前手势轴向。
  return CursorForRegion(IsResizing() ? ActiveRegion : HitTest(args));
}

UIEventReply ResizeBehavior::OnMouseDown(MouseMovArgs args) {
  if (args.Button != MouseButton::Left)
    return UIEventReply::Ignored;
  ActiveRegion = HitTest(args);
  if (ActiveRegion == ResizeRegion::None)
    return UIEventReply::Ignored;

  auto *owner = GetOwner();
  CurrentLeft = YGNodeLayoutGetLeft(owner->GetYogaNode());
  CurrentTop = YGNodeLayoutGetTop(owner->GetYogaNode());
  CurrentWidth = owner->GetLayoutWidth();
  CurrentHeight = owner->GetLayoutHeight();
  return UIEventReply::Capture;
}

bool ResizeBehavior::OnMouseDrag(MouseMovArgs args) {
  auto *owner = GetOwner();
  if (!owner || !IsResizing())
    return false;
  if (args.DeltaX == 0 && args.DeltaY == 0)
    return true;

  const bool changesLeft = ActiveRegion == ResizeRegion::Left ||
                           ActiveRegion == ResizeRegion::TopLeft ||
                           ActiveRegion == ResizeRegion::BottomLeft;
  const bool changesRight = ActiveRegion == ResizeRegion::Right ||
                            ActiveRegion == ResizeRegion::TopRight ||
                            ActiveRegion == ResizeRegion::BottomRight;
  const bool changesTop = ActiveRegion == ResizeRegion::Top ||
                          ActiveRegion == ResizeRegion::TopLeft ||
                          ActiveRegion == ResizeRegion::TopRight;
  const bool changesBottom = ActiveRegion == ResizeRegion::Bottom ||
                             ActiveRegion == ResizeRegion::BottomLeft ||
                             ActiveRegion == ResizeRegion::BottomRight;
  const float deltaX = static_cast<float>(args.DeltaX);
  const float deltaY = static_cast<float>(args.DeltaY);

  if (changesLeft) {
    const float nextWidth =
        (std::max)(Properties.MinWidth, CurrentWidth - deltaX);
    // 达到最小尺寸后只移动实际被尺寸吸收的距离，从而固定对侧边界。
    CurrentLeft += CurrentWidth - nextWidth;
    CurrentWidth = nextWidth;
  }
  if (changesRight)
    CurrentWidth = (std::max)(Properties.MinWidth, CurrentWidth + deltaX);
  if (changesTop) {
    const float nextHeight =
        (std::max)(Properties.MinHeight, CurrentHeight - deltaY);
    CurrentTop += CurrentHeight - nextHeight;
    CurrentHeight = nextHeight;
  }
  if (changesBottom)
    CurrentHeight = (std::max)(Properties.MinHeight, CurrentHeight + deltaY);

  auto yogaNode = owner->GetYogaNode();
  if (!InteractiveGeometry) {
    // 与拖拽使用相同的流式到绝对定位转换，避免主题 Margin 造成首次跳变。
    YGNodeStyleSetMargin(yogaNode, YGEdgeAll, 0.0f);
    InteractiveGeometry = true;
  }
  YGNodeStyleSetPositionType(yogaNode, YGPositionTypeAbsolute);
  YGNodeStyleSetPosition(yogaNode, YGEdgeLeft, CurrentLeft);
  YGNodeStyleSetPosition(yogaNode, YGEdgeTop, CurrentTop);
  YGNodeStyleSetWidth(yogaNode, CurrentWidth);
  YGNodeStyleSetHeight(yogaNode, CurrentHeight);
  YGNodeStyleSetFlexGrow(yogaNode, 0.0f);
  YGNodeStyleSetFlexShrink(yogaNode, 0.0f);
  return true;
}

bool ResizeBehavior::OnMouseUp(MouseMovArgs) {
  const bool handled = IsResizing();
  ActiveRegion = ResizeRegion::None;
  return handled;
}

bool ResizeBehavior::SetProperty(const std::string &name,
                                 const std::string &value) {
  if (name == "Resizable" || name == "ResizeEnabled")
    return ParseBoolean(value, Properties.Enabled);
  if (name == "ResizeBorder") {
    Properties.Border = (std::max)(0.0f, std::strtof(value.c_str(), nullptr));
    return true;
  }
  if (name == "MinWidth") {
    Properties.MinWidth = (std::max)(1.0f, std::strtof(value.c_str(), nullptr));
    ApplyMinimumSize();
    return true;
  }
  if (name == "MinHeight") {
    Properties.MinHeight =
        (std::max)(1.0f, std::strtof(value.c_str(), nullptr));
    ApplyMinimumSize();
    return true;
  }
  return false;
}
