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
private:
  BaseNode *Handle = nullptr;
  bool Dragging = false;
  bool GestureMoved = false;
  bool InteractiveGeometry = false;
  bool PreviewOnly = false;
  float CurrentLeft = 0.0f;
  float CurrentTop = 0.0f;
  float CurrentWidth = 0.0f;
  float CurrentHeight = 0.0f;
public:
  DragProperty Properties;
  static constexpr int DefaultPriority = 100;

  DragBehavior() : IBehavior(DefaultPriority) {}
  bool SetProperty(const std::string &name, const std::string &value) override;
  void SetHandle(BaseNode *handle) { Handle = handle; }

  bool IsDragging() const { return Dragging; }
  /** 区分标题单击与已经产生位移的真实拖拽，供日志和 Dock 生命周期使用。 */
  bool HasGestureMoved() const { return GestureMoved; }
  bool HasInteractiveGeometry() const { return InteractiveGeometry; }

  EventReply OnMouseDown(MouseMovArgs args) override;
  EventReply OnMouseDrag(MouseMovArgs args) override;
  EventReply OnMouseUp(MouseMovArgs args) override;
  /** 仅推进手势状态，由外部拖放系统绘制预览，不修改宿主节点布局。 */
  void SetPreviewOnly(bool previewOnly) { PreviewOnly = previewOnly; }
  void OnPointerCaptureLost() override {
    Dragging = false;
    GestureMoved = false;
  }
};

} // namespace z8::ui
