#include "UI/Behavior/DragBehavior.h"

#include "UI/Layout/BaseNode.h"
#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"

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

UIEventReply DragBehavior::OnMouseDown(MouseMovArgs args) {
  auto *owner = GetOwner();
  if (!owner || !Properties.Enabled || args.Button != MouseButton::Left ||
      !owner->Contains(args))
    return UIEventReply::Ignored;

  const bool inAllowedRegion = Properties.Region == DragRegion::Anywhere ||
                               (Handle && Handle->Contains(args));
  if (!inAllowedRegion)
    return UIEventReply::Ignored;

  // 快照 Yoga 的父空间坐标与已解析尺寸，使整个手势只累加相对位移，避免布局
  // 在两帧之间重新测量后造成拖动反馈跳变。
  CurrentLeft = YGNodeLayoutGetLeft(owner->GetYogaNode());
  CurrentTop = YGNodeLayoutGetTop(owner->GetYogaNode());
  CurrentWidth = owner->GetLayoutWidth();
  CurrentHeight = owner->GetLayoutHeight();
  Dragging = true;
  return UIEventReply::Capture;
}

bool DragBehavior::OnMouseDrag(MouseMovArgs args) {
  auto *owner = GetOwner();
  if (!owner || !Dragging)
    return false;
  if (args.DeltaX == 0 && args.DeltaY == 0)
    return true;

  CurrentLeft += static_cast<float>(args.DeltaX);
  CurrentTop += static_cast<float>(args.DeltaY);
  auto yogaNode = owner->GetYogaNode();
  if (!InteractiveGeometry) {
    // 流式位置已经包含 Margin；转换为绝对定位时清除它，防止首次拖动跳变。
    YGNodeStyleSetMargin(yogaNode, YGEdgeAll, 0.0f);
    InteractiveGeometry = true;
  }
  YGNodeStyleSetPositionType(yogaNode, YGPositionTypeAbsolute);
  YGNodeStyleSetPosition(yogaNode, YGEdgeLeft, CurrentLeft);
  YGNodeStyleSetPosition(yogaNode, YGEdgeTop, CurrentTop);
  // 脱离 Flex 流时固化完整布局框并关闭伸缩，否则 Yoga 会按内容重新测量，
  // 表现为第一次拖动时控件尺寸突然变化。
  YGNodeStyleSetWidth(yogaNode, CurrentWidth);
  YGNodeStyleSetHeight(yogaNode, CurrentHeight);
  YGNodeStyleSetFlexGrow(yogaNode, 0.0f);
  YGNodeStyleSetFlexShrink(yogaNode, 0.0f);
  return true;
}

bool DragBehavior::OnMouseUp(MouseMovArgs) {
  const bool handled = Dragging;
  Dragging = false;
  return handled;
}

bool DragBehavior::SetProperty(const std::string &name,
                               const std::string &value) {
  if (name == "Draggable" || name == "DragEnabled")
    return ParseBoolean(value, Properties.Enabled);
  if (name != "DragRegion")
    return false;
  if (value == "TitleBar")
    Properties.Region = DragRegion::TitleBar;
  else if (value == "Anywhere")
    Properties.Region = DragRegion::Anywhere;
  else
    return false;
  return true;
}
