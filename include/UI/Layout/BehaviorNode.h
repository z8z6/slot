#pragma once

#include "Core/Event.h"
#include "UI/Behavior/IBehavior.h"
#include "UI/Layout/BaseNode.h"

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace z8::ui {

/**
 * 选择加入交互能力的节点层
 */
class BehaviorNode : public BaseNode, public EventTarget {
public:
  // 非视觉交互容器必须显式开启；根 DockSpace 不应吞掉整个窗口的场景输入。
  bool HitTestVisible = false;
  /** 指针当前位于控件命中区域；只用于视觉反馈，不改变事件路由。 */
  bool Hovered = false;
  /** 左键手势尚未结束；捕获移出控件后仍保持 Pressed 视觉。 */
  bool Pressed = false;
  /** 控件是否可取得键盘焦点；普通容器保持 false，避免点击面板吞掉快捷键。 */
  bool Focusable = false;
  /** Layout 管理的唯一键盘焦点状态，不与 Hover/Pressed 生命周期混用。 */
  bool Focused = false;
  std::vector<std::unique_ptr<IBehavior>> Behaviors;
  IBehavior *CapturedBehavior = nullptr;

  ~BehaviorNode() override;
  bool SetProperty(const std::string &name, const std::string &value) override;

  EventReply DispatchMouseDown(const MouseMovArgs &args);
  bool DispatchMouseMove(const MouseMovArgs &args);
  bool DispatchMouseDrag(const MouseMovArgs &args);
  bool DispatchMouseUp(const MouseMovArgs &args);
  bool DispatchMouseWheel(MouseWheelArgs args);
  MouseCursor QueryMouseCursor(const MouseMovArgs &args) const;
  void DispatchAfterLayout() override;
  void DispatchBeforeLayout(float width, float height) const;
  void DispatchDragStarted(const MouseMovArgs &args) const;
  void DispatchDragCompleted(const MouseMovArgs &args) const;
  void CancelPointerCapture();

  IBehavior *AddBehavior(std::unique_ptr<IBehavior> behavior);
  bool RemoveBehavior(IBehavior *behavior);
  /** 由 Layout 的唯一命中结果更新 Hover，避免每个控件重复查询鼠标位置。 */
  void SetHovered(bool hovered);
  /** 由 Layout 绑定到一次按下/抬起生命周期，不与 Selected 或 Focused 混用。 */
  void SetPressed(bool pressed);
  /** 仅由 Layout 的唯一焦点槽调用，控件据此更新边框和插入光标。 */
  void SetFocused(bool focused);

  template <typename T, typename... Args> T *AddBehavior(Args &&...args) {
    static_assert(std::is_base_of_v<IBehavior, T>);
    auto behavior = std::make_unique<T>(std::forward<Args>(args)...);
    auto *observer = behavior.get();
    AddBehavior(std::move(behavior));
    return observer;
  }

  template <typename T> T *GetBehavior() {
    static_assert(std::is_base_of_v<IBehavior, T>);
    for (auto &behavior : Behaviors)
      if (auto *result = dynamic_cast<T *>(behavior.get()))
        return result;
    return nullptr;
  }

  template <typename T> const T *GetBehavior() const {
    static_assert(std::is_base_of_v<IBehavior, T>);
    for (const auto &behavior : Behaviors)
      if (auto *result = dynamic_cast<const T *>(behavior.get()))
        return result;
    return nullptr;
  }

protected:
  /** 派生控件在状态变化时只重算主题色，不得改变布局或真实命中区域。 */
  virtual void OnVisualStateChanged() {}
  void ReleaseBehaviors();
};

} // namespace z8::ui
