#include "UI/Behavior/DragBehavior.h"

#include "UI/Behavior/DockBehavior.h"
#include "UI/Layout/BaseNode.h"
#include "UI/Layout/BehaviorNode.h"

#include <algorithm>

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
                               (Handle && Handle->Contains(args));
  if (!inAllowedRegion)
    return EventReply::Ignored;

  // 快照父空间坐标与已解析尺寸，使整个手势只累加相对位移，避免布局
  // 在两帧之间重新测量后造成拖动反馈跳变。
  CurrentLeft = owner->Computed.Left;
  CurrentTop = owner->Computed.Top;
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
    // 只在产生真实位移后通知，避免单击标题栏触发停靠重排。
    owner->DispatchDragStarted(args);
  }

  // Panel Tab 是拖放句柄而不是可移动窗口；它只驱动 DockWorkspace 的预览，
  // 保持源 Group 的图标、标题和页签几何不变，直到 MouseUp 原子提交成员迁移。
  if (PreviewOnly)
    return EventReply::Handled;

  // Docked Panel 的拖动由 Layout 中的唯一 DockWorkspace 会话维护。拖动阶段
  // 只更新预览，不能改 Style 或 DockTree；MouseUp 才提交 Dock/Floating 结果。
  if (const auto *dock = owner->GetBehavior<DockBehavior>();
      dock && dock->Properties.Enabled)
    return EventReply::Handled;

  CurrentLeft += static_cast<float>(args.DeltaX);
  CurrentTop += static_cast<float>(args.DeltaY);
  if (const auto *parent = owner->Parent) {
    // 至少保留标题栏的一部分在父工作区内，否则释放后用户无法再次命中拖拽句柄。
    const float handleHeight = Handle ? Handle->Height : 24.0f;
    constexpr float reachableWidth = 48.0f;
    CurrentLeft = (std::clamp)(CurrentLeft,
        -CurrentWidth + reachableWidth,
        (std::max)(0.0f, parent->Width - reachableWidth));
    CurrentTop = (std::clamp)(CurrentTop, 0.0f,
        (std::max)(0.0f, parent->Height - handleHeight));
  }
  if (!InteractiveGeometry) {
    // 流式位置已经包含 Margin；转换为绝对定位时清除它，防止首次拖动跳变。
    owner->Style.Margin = 0.0f;
    InteractiveGeometry = true;
  }
  owner->Style.Position = PositionType::Absolute;
  owner->Style.Left = CurrentLeft;
  owner->Style.Top = CurrentTop;
  // 脱离 Flex 流时固化完整布局框并关闭伸缩，否则内容测量会改变交互框，
  // 表现为第一次拖动时控件尺寸突然变化。
  owner->Style.Width = CurrentWidth;
  owner->Style.Height = CurrentHeight;
  owner->Style.FlexGrow = 0.0f;
  owner->Style.FlexShrink = 0.0f;
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
