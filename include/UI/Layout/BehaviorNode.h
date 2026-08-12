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
 * 选择加入交互能力的节点层。
 *
 * 纯布局 BaseNode 不承担事件 ABI、行为容器和捕获状态；需要组合交互策略的
 * 控件显式继承本类，从类型上标出 Layout 可以路由到的节点。
 */
class BehaviorNode : public BaseNode, public EventTarget {
public:
  // 非视觉交互容器必须显式开启；根 DockSpace 不应吞掉整个窗口的场景输入。
  bool HitTestVisible = false;
  std::vector<std::unique_ptr<IBehavior>> Behaviors;
  IBehavior *CapturedBehavior = nullptr;

  ~BehaviorNode() override;
  bool SetProperty(const std::string &name, const std::string &value) override;


  EventReply DispatchMouseDown(MouseMovArgs args);
  bool DispatchMouseMove(MouseMovArgs args);
  bool DispatchMouseDrag(MouseMovArgs args);
  bool DispatchMouseUp(MouseMovArgs args);
  bool DispatchMouseWheel(MouseWheelArgs args);
  MouseCursor QueryMouseCursor(MouseMovArgs args) const;
  void DispatchLayoutUpdated();
  void DispatchBeforeLayout(float width, float height);
  void DispatchDragStarted(MouseMovArgs args);
  void DispatchDragCompleted(MouseMovArgs args);
  void CancelPointerCapture();

  IBehavior *AddBehavior(std::unique_ptr<IBehavior> behavior);
  bool RemoveBehavior(IBehavior *behavior);

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
  void ReleaseBehaviors();
};

} // namespace z8::ui
