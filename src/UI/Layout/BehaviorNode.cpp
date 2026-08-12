#include "UI/Layout/BehaviorNode.h"

#include <algorithm>

using namespace z8;
using namespace z8::ui;

BehaviorNode::~BehaviorNode() { ReleaseBehaviors(); }

void BehaviorNode::ReleaseBehaviors() {
  CancelPointerCapture();
  for (const auto &behavior : Behaviors) {
    behavior->OnDetached();
    behavior->Owner = nullptr;
  }
  Behaviors.clear();
}

IBehavior *BehaviorNode::AddBehavior(std::unique_ptr<IBehavior> behavior) {
  if (!behavior || behavior->Owner)
    return nullptr;
  auto *result = behavior.get();
  result->Owner = this;
  Behaviors.push_back(std::move(behavior));
  // 稳定排序使同优先级的声明顺序成为可预测的冲突规则。
  std::stable_sort(Behaviors.begin(), Behaviors.end(),
                   [](const auto &left, const auto &right) {
                     return left->Priority > right->Priority;
                   });
  result->OnAttached();
  return result;
}

bool BehaviorNode::RemoveBehavior(IBehavior *behavior) {
  const auto iterator =
      std::find_if(Behaviors.begin(), Behaviors.end(), [behavior](const auto &v) {
        return v.get() == behavior;
      });
  if (iterator == Behaviors.end())
    return false;
  if (CapturedBehavior == behavior) {
    behavior->OnPointerCaptureLost();
    CapturedBehavior = nullptr;
  }
  (*iterator)->OnDetached();
  (*iterator)->Owner = nullptr;
  Behaviors.erase(iterator);
  return true;
}

bool BehaviorNode::SetProperty(const std::string &name,
                               const std::string &value) {
  for (const auto &behavior : Behaviors)
    if (behavior->SetProperty(name, value))
      return true;
  return BaseNode::SetProperty(name, value);
}

// 设置 CapturedBehavior
EventReply BehaviorNode::DispatchMouseDown(const MouseMovArgs &args) {
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

bool BehaviorNode::DispatchMouseMove(const MouseMovArgs &args) {
  for (const auto &behavior : Behaviors)
    if (behavior->OnMouseMove(args) != EventReply::Ignored)
      return true;
  return OnMouseMove(args) != EventReply::Ignored;
}

// 只有 CapturedBehavior
bool BehaviorNode::DispatchMouseDrag(const MouseMovArgs &args) {
  return (CapturedBehavior ? CapturedBehavior->OnMouseDrag(args)
                           : OnMouseDrag(args)) != EventReply::Ignored;
}

// 清空 CapturedBehavior
bool BehaviorNode::DispatchMouseUp(const MouseMovArgs &args) {
  if (!CapturedBehavior)
    return OnMouseUp(args) != EventReply::Ignored;
  auto *behavior = CapturedBehavior;
  CapturedBehavior = nullptr;
  return behavior->OnMouseUp(args) != EventReply::Ignored;
}

bool BehaviorNode::DispatchMouseWheel(const MouseWheelArgs args) {
  for (const auto &behavior : Behaviors)
    if (behavior->OnMouseWheel(args) != EventReply::Ignored)
      return true;
  return OnMouseWheel(args) != EventReply::Ignored;
}

z8::MouseCursor BehaviorNode::QueryMouseCursor(const MouseMovArgs &args) const {
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

void BehaviorNode::DispatchAfterLayout() {
  for (const auto &behavior : Behaviors)
    behavior->OnAfterLayout();
  BaseNode::DispatchAfterLayout();
}

void BehaviorNode::DispatchBeforeLayout(float width, float height) const {
  for (const auto &behavior : Behaviors)
    behavior->OnBeforeLayout(width, height);
}

void BehaviorNode::DispatchDragStarted(const MouseMovArgs &args) const {
  for (const auto &behavior : Behaviors)
    behavior->OnDragStarted(args);
}

void BehaviorNode::DispatchDragCompleted(const MouseMovArgs &args) const {
  for (const auto &behavior : Behaviors)
    behavior->OnDragCompleted(args);
}

void BehaviorNode::CancelPointerCapture() {
  if (CapturedBehavior) {
    CapturedBehavior->OnPointerCaptureLost();
    CapturedBehavior = nullptr;
  }
  OnPointerCaptureLost();
}
