#pragma once

#include "Core/Event.h"
#include "UI/Property/IProperty.h"

namespace z8::ui {

class BaseNode;

/**
 * 行为处理一次指针按下后的路由结果。
 *
 * Handled 只阻止事件继续冒泡；Capture 还要求后续拖动和抬起固定送回同一
 * Behavior。显式区分二者，可避免按钮点击之类的瞬时事件被误当成长手势。
 */
enum class UIEventReply { Ignored, Handled, Capture };

/**
 * 可挂载到任意 BaseNode 的交互行为基类。
 *
 * Behavior 只观察宿主，不拥有节点或视觉子树。宿主先销毁 Behavior，再销毁
 * UIObject、子节点和 Yoga 句柄，因此派生类可以安全缓存受宿主生命周期约束的
 * 非拥有指针，但不得把这些指针传递到 UI 树之外长期保存。
 */
class UIBehavior : public IProperty {
public:
  explicit UIBehavior(int priority = 0) : Priority(priority) {}
  ~UIBehavior() override = default;

  UIBehavior(const UIBehavior &) = delete;
  UIBehavior &operator=(const UIBehavior &) = delete;

  BaseNode *GetOwner() const { return Owner; }
  int GetPriority() const { return Priority; }

  /** 输入、光标和布局钩子默认忽略；派生行为只覆写所消费的阶段。 */
  virtual UIEventReply OnMouseDown(MouseMovArgs) {
    return UIEventReply::Ignored;
  }
  virtual bool OnMouseDrag(MouseMovArgs) { return false; }
  virtual bool OnMouseUp(MouseMovArgs) { return false; }
  virtual bool OnMouseWheel(MouseWheelArgs) { return false; }
  virtual MouseCursor GetMouseCursor(MouseMovArgs) const {
    return MouseCursor::Arrow;
  }
  virtual void OnLayoutUpdated() {}
  /** 路由器因拓扑变化取消捕获时，行为必须回到空闲状态。 */
  virtual void OnCaptureLost() {}
  /** 行为属性与节点属性共享字符串入口，返回 false 继续查询下一能力。 */
  bool SetProperty(const std::string &, const std::string &) override {
    return false;
  }

protected:
  /** 宿主建立后调用；派生类在此同步依赖 Owner 的约束。 */
  virtual void OnAttached() {}
  /** 卸载前调用；用于断开由行为安装到复合视觉上的回调。 */
  virtual void OnDetached() {}

private:
  friend class BaseNode;
  BaseNode *Owner = nullptr;
  int Priority = 0;
};

} // namespace z8::ui
