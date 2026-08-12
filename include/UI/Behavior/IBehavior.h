#pragma once

#include "Core/Event.h"
#include "UI/Property/IProperty.h"

namespace z8::ui {

class BaseNode;
class BehaviorNode;

/**
 * 可挂载到任意 BehaviorNode 的交互行为基类。
 *
 * Behavior 只观察宿主，不拥有节点或视觉子树。
 * 宿主先销毁 Behavior，再销毁UIObject、子节点和 Yoga 句柄
 */
class IBehavior : public IProperty, public EventTarget {
public:
  BehaviorNode *Owner = nullptr;
  int Priority = 0;

  explicit IBehavior(int priority = 0) : Priority(priority) {}
  ~IBehavior() override = default;
  IBehavior(const IBehavior &) = delete;
  IBehavior &operator=(const IBehavior &) = delete;

  // Yoga 计算前更新容器约束；仅布局类 Behavior 应覆写该阶段。
  virtual void OnBeforeLayout(float, float) {}
  virtual void OnAfterLayout() {}

  // 观察同一宿主上的有效拖拽生命周期，用于停靠等正交策略。
  virtual void OnDragStarted(MouseMovArgs) {}
  virtual void OnDragCompleted(MouseMovArgs) {}
  virtual void OnAttached() {}
  virtual void OnDetached() {}
};

} // namespace z8::ui
