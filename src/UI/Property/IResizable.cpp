#include "UI/Property/IResizable.h"

#include "UI/Layout/BaseNode.h"
#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"

#include <algorithm>

using namespace z8::ui;

z8::MouseCursor IResizable::CursorForRegion(ResizeRegion region) {
  switch (region) {
  case ResizeRegion::Left:
  case ResizeRegion::Right: return MouseCursor::SizeHorizontal;
  case ResizeRegion::Top:
  case ResizeRegion::Bottom: return MouseCursor::SizeVertical;
  case ResizeRegion::TopLeft:
  case ResizeRegion::BottomRight:
    return MouseCursor::SizeDiagonalNorthwestSoutheast;
  case ResizeRegion::TopRight:
  case ResizeRegion::BottomLeft:
    return MouseCursor::SizeDiagonalNortheastSouthwest;
  default: return MouseCursor::Arrow;
  }
}

ResizeRegion IResizable::HitTestResize(const BaseNode* node,
                                       MouseMovArgs args) const {
  if (!ResizeProperties.Enabled || !node->Contains(args))
    return ResizeRegion::None;
  const float x = static_cast<float>(args.X);
  const float y = static_cast<float>(args.Y);
  const bool left = x - node->GetLayoutX() <= ResizeProperties.Border;
  const bool right = node->GetLayoutX() + node->GetLayoutWidth() - x <=
                     ResizeProperties.Border;
  const bool top = y - node->GetLayoutY() <= ResizeProperties.Border;
  const bool bottom = node->GetLayoutY() + node->GetLayoutHeight() - y <=
                      ResizeProperties.Border;

  if (top && left) return ResizeRegion::TopLeft;
  if (top && right) return ResizeRegion::TopRight;
  if (bottom && left) return ResizeRegion::BottomLeft;
  if (bottom && right) return ResizeRegion::BottomRight;
  if (left) return ResizeRegion::Left;
  if (right) return ResizeRegion::Right;
  if (top) return ResizeRegion::Top;
  if (bottom) return ResizeRegion::Bottom;
  return ResizeRegion::None;
}

z8::MouseCursor IResizable::GetResizeCursor(const BaseNode* node,
                                            MouseMovArgs args) const {
  // 捕获期间保持手势开始边对应的光标，即使鼠标已离开原始控件边界。
  return CursorForRegion(IsResizing() ? ActiveRegion
                                      : HitTestResize(node, args));
}

bool IResizable::BeginResize(BaseNode* node, MouseMovArgs args) {
  if (args.Button != MouseButton::Left) return false;
  ActiveRegion = HitTestResize(node, args);
  if (ActiveRegion == ResizeRegion::None) return false;

  auto yogaNode = node->GetYogaNode();
  CurrentLeft = YGNodeLayoutGetLeft(yogaNode);
  CurrentTop = YGNodeLayoutGetTop(yogaNode);
  CurrentWidth = node->GetLayoutWidth();
  CurrentHeight = node->GetLayoutHeight();
  return true;
}

bool IResizable::UpdateResize(BaseNode* node, MouseMovArgs args) {
  if (!IsResizing()) return false;
  if (args.DeltaX == 0 && args.DeltaY == 0) return true;

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
    const float nextWidth = (std::max)(ResizeProperties.MinWidth,
                                       CurrentWidth - deltaX);
    // 最小尺寸处只移动尺寸实际吸收的距离，从而保持对侧边界不漂移。
    CurrentLeft += CurrentWidth - nextWidth;
    CurrentWidth = nextWidth;
  }
  if (changesRight)
    CurrentWidth = (std::max)(ResizeProperties.MinWidth,
                              CurrentWidth + deltaX);
  if (changesTop) {
    const float nextHeight = (std::max)(ResizeProperties.MinHeight,
                                        CurrentHeight - deltaY);
    CurrentTop += CurrentHeight - nextHeight;
    CurrentHeight = nextHeight;
  }
  if (changesBottom)
    CurrentHeight = (std::max)(ResizeProperties.MinHeight,
                               CurrentHeight + deltaY);

  auto yogaNode = node->GetYogaNode();
  if (!InteractiveGeometry) {
    // 与拖拽保持同一转换规则，避免带主题 Margin 的控件首次拉伸时跳动。
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

bool IResizable::EndResize() {
  const bool handled = IsResizing();
  ActiveRegion = ResizeRegion::None;
  return handled;
}
