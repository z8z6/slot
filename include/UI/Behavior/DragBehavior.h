#pragma once

#include "UI/Behavior/IBehavior.h"

namespace z8::ui {

class BaseNode;

/** 控件允许从标题栏或任意内部区域启动移动。 */
enum class DragRegion { TitleBar, Anywhere };

/** 可序列化的拖拽配置；运行时手势状态只由 DragBehavior 维护。 */
struct DragProperty {
  bool Enabled = true;
  DragRegion Region = DragRegion::TitleBar;
};

/**
 * 将宿主从 Flex 流转换为绝对定位并持续移动的独立手势组件。
 *
 * Handle 是可选的非拥有观察指针。TitleBar 模式只允许从 Handle 开始；
 * Anywhere 模式使用宿主命中框。组件不关心 Panel 或其他具体控件类型。
 */
class DragBehavior final : public IBehavior {
public:
  static constexpr int DefaultPriority = 100;

  DragBehavior() : IBehavior(DefaultPriority) {}

  /** 简单配置公开，避免为纯数据重复包装访问器。 */
  DragProperty Properties;

  void SetHandle(BaseNode *handle) { Handle = handle; }
  bool IsDragging() const { return Dragging; }
  bool HasInteractiveGeometry() const { return InteractiveGeometry; }

  /** 三个事件共同实现一次捕获式拖动；属性入口处理声明层名称。 */
  EventReply OnMouseDown(MouseMovArgs args) override;
  EventReply OnMouseDrag(MouseMovArgs args) override;
  EventReply OnMouseUp(MouseMovArgs args) override;
  void OnPointerCaptureLost() override {
    Dragging = false;
    GestureMoved = false;
  }
  bool SetProperty(const std::string &name, const std::string &value) override;

private:
  BaseNode *Handle = nullptr;
  bool Dragging = false;
  bool GestureMoved = false;
  bool InteractiveGeometry = false;
  float CurrentLeft = 0.0f;
  float CurrentTop = 0.0f;
  float CurrentWidth = 0.0f;
  float CurrentHeight = 0.0f;
};

} // namespace z8::ui
