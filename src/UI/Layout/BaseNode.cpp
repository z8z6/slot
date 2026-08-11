//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/BaseNode.h"

#include "yoga/YGNodeStyle.h"
#include "yoga/node/Node.h"

#include <algorithm>
#include <cstdlib>

using namespace z8::ui;
using z8::EventReply;

BaseNode::BaseNode() : Node(YGNodeNew()) {
  // 保存 BaseNode 指针到 YGNode, 这样可以双向查询
  YGNodeSetContext(Node, this);
  YGNodeStyleSetFlexGrow(Node, 1.0f);
  YGNodeStyleSetFlexShrink(Node, 1.0f);
}

BaseNode::~BaseNode() {
  ReleaseBehaviors();
  YGNodeRemoveAllChildren(Node);
  Children.clear();
  YGNodeFree(Node);
  Node = nullptr;
}

void BaseNode::ReleaseBehaviors() {
  // Behavior 可能观察内部视觉节点，必须在视觉、子树和 Yoga 几何失效前释放。
  CancelPointerCapture();
  for (const auto &behavior : Behaviors) {
    behavior->OnDetached();
    behavior->Owner = nullptr;
  }
  Behaviors.clear();
}

bool BaseNode::Contains(float x, float y) const {
  return Visible && x >= Left && y >= Top && x <= Left + Width &&
         y <= Top + Height && x >= VisibleClip.x && y >= VisibleClip.y &&
         x <= VisibleClip.z && y <= VisibleClip.w;
}
bool BaseNode::Contains(MouseMovArgs args) const {
  return Contains(static_cast<float>(args.X), static_cast<float>(args.Y));
}

UIBehavior *BaseNode::AddBehavior(std::unique_ptr<UIBehavior> behavior) {
  if (!behavior || behavior->Owner)
    return nullptr;
  auto *result = behavior.get();
  result->Owner = this;
  Behaviors.push_back(std::move(behavior));
  // stable_sort 让同优先级的声明顺序成为稳定且可预测的冲突规则。
  std::stable_sort(Behaviors.begin(), Behaviors.end(),
                   [](const auto &left, const auto &right) {
                     return left->GetPriority() > right->GetPriority();
                   });
  result->OnAttached();
  return result;
}

bool BaseNode::RemoveBehavior(UIBehavior *behavior) {
  if (!behavior)
    return false;
  const auto iterator = std::find_if(Behaviors.begin(), Behaviors.end(),
                                     [behavior](const auto &candidate) {
                                       return candidate.get() == behavior;
                                     });
  if (iterator == Behaviors.end())
    return false;
  if (CapturedBehavior == behavior) {
    behavior->OnCaptureLost();
    CapturedBehavior = nullptr;
  }
  (*iterator)->OnDetached();
  (*iterator)->Owner = nullptr;
  Behaviors.erase(iterator);
  return true;
}

EventReply BaseNode::DispatchMouseDown(MouseMovArgs args) {
  CapturedBehavior = nullptr;
  for (const auto &behavior : Behaviors) {
    const auto reply = behavior->OnMouseDown(args);
    if (reply == EventReply::Capture)
      CapturedBehavior = behavior.get();
    if (reply != EventReply::Ignored)
      return reply;
  }
  return OnMouseDown(args);
}

bool BaseNode::DispatchMouseMove(MouseMovArgs args) {
  for (const auto &behavior : Behaviors)
    if (behavior->OnMouseMove(args) != EventReply::Ignored)
      return true;
  return OnMouseMove(args) != EventReply::Ignored;
}

bool BaseNode::DispatchMouseDrag(MouseMovArgs args) {
  const auto reply = CapturedBehavior ? CapturedBehavior->OnMouseDrag(args)
                                      : OnMouseDrag(args);
  return reply != EventReply::Ignored;
}

bool BaseNode::DispatchMouseUp(MouseMovArgs args) {
  if (!CapturedBehavior)
    return OnMouseUp(args) != EventReply::Ignored;
  auto *behavior = CapturedBehavior;
  const bool handled = behavior->OnMouseUp(args) != EventReply::Ignored;
  CapturedBehavior = nullptr;
  return handled;
}

bool BaseNode::DispatchMouseWheel(MouseWheelArgs args) {
  for (const auto &behavior : Behaviors)
    if (behavior->OnMouseWheel(args) != EventReply::Ignored)
      return true;
  return OnMouseWheel(args) != EventReply::Ignored;
}

z8::MouseCursor BaseNode::QueryMouseCursor(MouseMovArgs args) const {
  if (CapturedBehavior) {
    const auto cursor = CapturedBehavior->GetMouseCursor(args);
    if (cursor != MouseCursor::Arrow)
      return cursor;
  }
  for (const auto &behavior : Behaviors) {
    if (behavior.get() == CapturedBehavior)
      continue;
    const auto cursor = behavior->GetMouseCursor(args);
    if (cursor != MouseCursor::Arrow)
      return cursor;
  }
  return GetMouseCursor(args);
}

void BaseNode::DispatchLayoutUpdated() {
  for (const auto &behavior : Behaviors)
    behavior->OnLayoutUpdated();
  OnLayoutUpdated();
}

void BaseNode::DispatchBeforeLayout(float width, float height) {
  // 行为按既定优先级更新约束，使多个布局策略的覆盖顺序与输入仲裁一致。
  for (const auto &behavior : Behaviors)
    behavior->OnBeforeLayout(width, height);
}

void BaseNode::DispatchDragStarted(MouseMovArgs args) {
  for (const auto &behavior : Behaviors)
    behavior->OnDragStarted(args);
}

void BaseNode::DispatchDragCompleted(MouseMovArgs args) {
  for (const auto &behavior : Behaviors)
    behavior->OnDragCompleted(args);
}

void BaseNode::CancelPointerCapture() {
  if (CapturedBehavior) {
    CapturedBehavior->OnCaptureLost();
    CapturedBehavior = nullptr;
  }
  OnPointerCaptureLost();
}

BaseNode *BaseNode::AddChild(std::unique_ptr<BaseNode> child) {
  if (!child)
    return nullptr;
  child->Parent = this;
  auto *result = child.get();
  YGNodeInsertChild(Node, child->Node, Children.size());
  Children.push_back(std::move(child));
  return result;
}

void BaseNode::RemoveChildrenFrom(size_t first) {
  if (first >= Children.size())
    return;
  for (size_t i = Children.size(); i > first; --i)
    YGNodeRemoveChild(Node, Children[i - 1]->Node);
  Children.erase(Children.begin() + static_cast<std::ptrdiff_t>(first),
                 Children.end());
}

BaseNode *BaseNode::ContentHost() { return this; }

const char *BaseNode::TypeName() const { return "UI"; }

bool BaseNode::SetProperty(const std::string &name, const std::string &value) {
  // 声明层无需知道能力属于控件还是外挂 Behavior；挂载即自动进入属性链。
  for (const auto &behavior : Behaviors)
    if (behavior->SetProperty(name, value))
      return true;
  const float number = std::strtof(value.c_str(), nullptr);
  if (name == "Id" || name == "Key" || name == "Name")
    Key = value;
  else if (name == "Width")
    YGNodeStyleSetWidth(Node, number);
  else if (name == "Height")
    YGNodeStyleSetHeight(Node, number);
  else if (name == "MinWidth")
    YGNodeStyleSetMinWidth(Node, number);
  else if (name == "MinHeight")
    YGNodeStyleSetMinHeight(Node, number);
  else if (name == "MaxWidth")
    YGNodeStyleSetMaxWidth(Node, number);
  else if (name == "MaxHeight")
    YGNodeStyleSetMaxHeight(Node, number);
  else if (name == "FlexGrow")
    YGNodeStyleSetFlexGrow(Node, number);
  else if (name == "FlexShrink")
    YGNodeStyleSetFlexShrink(Node, number);
  else if (name == "Margin")
    YGNodeStyleSetMargin(Node, YGEdgeAll, number);
  else if (name == "Padding")
    YGNodeStyleSetPadding(Node, YGEdgeAll, number);
  else if (name == "Direction") {
    if (value == "Row")
      YGNodeStyleSetFlexDirection(Node, YGFlexDirectionRow);
    else if (value == "Column")
      YGNodeStyleSetFlexDirection(Node, YGFlexDirectionColumn);
    else
      return false;
  } else
    return false;
  return true;
}
