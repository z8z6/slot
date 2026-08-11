#include "UI/Property/IDraggable.h"

#include "UI/Layout/BaseNode.h"
#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"

using namespace z8::ui;

bool IDraggable::BeginDrag(BaseNode* node, MouseMovArgs args,
                           bool inAllowedRegion) {
  if (!DragProperties.Enabled || !inAllowedRegion ||
      args.Button != MouseButton::Left || !node->Contains(args))
    return false;

  CurrentLeft = YGNodeLayoutGetLeft(node->GetYogaNode());
  CurrentTop = YGNodeLayoutGetTop(node->GetYogaNode());
  CurrentWidth = node->GetLayoutWidth();
  CurrentHeight = node->GetLayoutHeight();
  Dragging = true;
  return true;
}

bool IDraggable::UpdateDrag(BaseNode* node, MouseMovArgs args) {
  if (!Dragging) return false;
  if (args.DeltaX == 0 && args.DeltaY == 0) return true;

  CurrentLeft += static_cast<float>(args.DeltaX);
  CurrentTop += static_cast<float>(args.DeltaY);
  auto yogaNode = node->GetYogaNode();
  if (!InteractiveGeometry) {
    // 流式位置已经包含 Margin；转换为绝对定位时清除它，防止首次拖动跳变。
    YGNodeStyleSetMargin(yogaNode, YGEdgeAll, 0.0f);
    InteractiveGeometry = true;
  }
  YGNodeStyleSetPositionType(yogaNode, YGPositionTypeAbsolute);
  YGNodeStyleSetPosition(yogaNode, YGEdgeLeft, CurrentLeft);
  YGNodeStyleSetPosition(yogaNode, YGEdgeTop, CurrentTop);
  // 脱离 Flex 流时必须固化完整布局框；只设置位置会让 Yoga 重新按内容计算
  // Panel 高度，表现为第一次拖动窗口突然缩小。
  YGNodeStyleSetWidth(yogaNode, CurrentWidth);
  YGNodeStyleSetHeight(yogaNode, CurrentHeight);
  // 绝对定位脱离 Flex 流，关闭伸缩使交互后的几何保持稳定。
  YGNodeStyleSetFlexGrow(yogaNode, 0.0f);
  YGNodeStyleSetFlexShrink(yogaNode, 0.0f);
  return true;
}

bool IDraggable::EndDrag() {
  const bool handled = Dragging;
  Dragging = false;
  return handled;
}
