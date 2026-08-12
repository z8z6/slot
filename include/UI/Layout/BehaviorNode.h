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
class BehaviorNode : public BaseNode,
                     public EventTarget {
public:
  // 非视觉交互容器必须显式开启；根 DockSpace 不应吞掉整个窗口的场景输入。
  bool HitTestVisible = false;
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
