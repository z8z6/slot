#pragma once

#include "Core/Event.h"
#include "UI/Property/IProperty.h"

namespace z8::ui {

class BaseNode;

/**
 * 可挂载到任意 BaseNode 的交互行为基类。
 *
 * Behavior 只观察宿主，不拥有节点或视觉子树。宿主先销毁 Behavior，再销毁
 * UIObject、子节点和 Yoga 句柄，因此派生类可以安全缓存受宿主生命周期约束的
 * 非拥有指针，但不得把这些指针传递到 UI 树之外长期保存。
 */
class IBehavior : public IProperty, public EventTarget {
public:
  explicit IBehavior(int priority = 0) : Priority(priority) {}
  ~IBehavior() override = default;

  IBehavior(const IBehavior &) = delete;
  IBehavior &operator=(const IBehavior &) = delete;

  BaseNode *GetOwner() const { return Owner; }
  int GetPriority() const { return Priority; }

  /** 布局钩子是 UI 扩展；输入、光标与捕获钩子复用 EventTarget 契约。 */
  virtual void OnLayoutUpdated() {}
  // Yoga 计算前更新容器约束；仅布局类 Behavior 应覆写该阶段。
  virtual void OnBeforeLayout(float, float) {}
  // 观察同一宿主上的有效拖拽生命周期，用于停靠等正交策略。
  virtual void OnDragStarted(MouseMovArgs) {}
  virtual void OnDragCompleted(MouseMovArgs) {}
  // 行为属性与节点属性共享字符串入口，返回 false 继续查询下一能力。
  bool SetProperty(const std::string &, const std::string &) override {
    return false;
  }

protected:
  // 宿主建立后调用；同步依赖 Owner 的约束
  virtual void OnAttached() {}
  // 卸载前调用
  virtual void OnDetached() {}

private:
  friend class BaseNode;
  BaseNode *Owner = nullptr;
  int Priority = 0;
};

} // namespace z8::ui
