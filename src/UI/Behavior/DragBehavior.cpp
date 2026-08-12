#include "UI/Behavior/DragBehavior.h"

#include "UI/Layout/BaseNode.h"
#include "UI/Layout/BehaviorNode.h"
#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"

using namespace z8::ui;
using z8::EventReply;

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

EventReply DragBehavior::OnMouseDown(MouseMovArgs args) {
  auto *owner = Owner;
  if (!owner || !Properties.Enabled || args.Button != MouseButton::Left ||
      !owner->Contains(static_cast<float>(args.X), static_cast<float>(args.Y)))
    return EventReply::Ignored;

  const bool inAllowedRegion = Properties.Region == DragRegion::Anywhere ||
                               (Handle && Handle->Contains(
                                              static_cast<float>(args.X),
                                              static_cast<float>(args.Y)));
  if (!inAllowedRegion)
    return EventReply::Ignored;

  // 快照 Yoga 的父空间坐标与已解析尺寸，使整个手势只累加相对位移，避免布局
  // 在两帧之间重新测量后造成拖动反馈跳变。
  CurrentLeft = YGNodeLayoutGetLeft(owner->Node);
  CurrentTop = YGNodeLayoutGetTop(owner->Node);
  CurrentWidth = owner->Width;
  CurrentHeight = owner->Height;
  Dragging = true;
  GestureMoved = false;
  return EventReply::Capture;
}

EventReply DragBehavior::OnMouseDrag(MouseMovArgs args) {
  auto *owner = Owner;
  if (!owner || !Dragging)
    return EventReply::Ignored;
  if (args.DeltaX == 0 && args.DeltaY == 0)
    return EventReply::Handled;

  if (!GestureMoved) {
    GestureMoved = true;
    // 只在产生真实位移后通知，避免单击标题栏把已停靠 Panel 变成浮动窗口。
    owner->DispatchDragStarted(args);
  }

  CurrentLeft += static_cast<float>(args.DeltaX);
  CurrentTop += static_cast<float>(args.DeltaY);
  auto yogaNode = owner->Node;
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
  return EventReply::Handled;
}

EventReply DragBehavior::OnMouseUp(MouseMovArgs args) {
  const bool handled = Dragging;
  if (Dragging && GestureMoved && Owner)
    Owner->DispatchDragCompleted(args);
  Dragging = false;
  GestureMoved = false;
  return handled ? EventReply::Handled : EventReply::Ignored;
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
