#pragma once

#include "UI/Behavior/UIBehavior.h"
#include "UI/Property/IDraggable.h"

namespace z8::ui {

class BaseNode;

/**
 * 将宿主从 Flex 流转换为绝对定位并持续移动的独立手势组件。
 *
 * Handle 是可选的非拥有观察指针。TitleBar 模式只允许从 Handle 开始；
 * Anywhere 模式使用宿主命中框。组件不关心 Panel 或其他具体控件类型。
 */
class DragBehavior final : public UIBehavior {
public:
  static constexpr int DefaultPriority = 100;

  DragBehavior() : UIBehavior(DefaultPriority) {}

  /** 配置与状态访问器不暴露 Yoga 细节，供兼容接口和检查器使用。 */
  const DragProperty &GetProperties() const { return Properties; }
  void SetProperties(const DragProperty &properties) {
    Properties = properties;
  }
  void SetHandle(BaseNode *handle) { Handle = handle; }
  bool IsDragging() const { return Dragging; }
  bool HasInteractiveGeometry() const { return InteractiveGeometry; }

  /** 三个事件共同实现一次捕获式拖动；属性入口处理声明层名称。 */
  UIEventReply OnMouseDown(MouseMovArgs args) override;
  bool OnMouseDrag(MouseMovArgs args) override;
  bool OnMouseUp(MouseMovArgs args) override;
  void OnCaptureLost() override { Dragging = false; }
  bool SetProperty(const std::string &name, const std::string &value) override;

private:
  DragProperty Properties;
  BaseNode *Handle = nullptr;
  bool Dragging = false;
  bool InteractiveGeometry = false;
  float CurrentLeft = 0.0f;
  float CurrentTop = 0.0f;
  float CurrentWidth = 0.0f;
  float CurrentHeight = 0.0f;
};

} // namespace z8::ui
