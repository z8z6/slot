#pragma once

#include "UI/Behavior/IBehavior.h"

namespace z8::ui {

/** 拉伸命中区域；角落同时改变两个轴。 */
enum class ResizeRegion {
  None,
  Left,
  Right,
  Top,
  Bottom,
  TopLeft,
  TopRight,
  BottomLeft,
  BottomRight
};

/** 可序列化的拉伸配置，边界宽度使用窗口客户区逻辑像素。 */
struct ResizeProperty {
  bool Enabled = true;
  float Border = 6.0f;
  float MinWidth = 120.0f;
  float MinHeight = 80.0f;
};

/**
 * 在宿主边界和四角提供尺寸调整的独立手势组件。
 *
 * 组件优先级高于 DragBehavior，保证重叠的标题栏边缘只启动一种几何状态机。
 */
class ResizeBehavior final : public IBehavior {
public:
  static constexpr int DefaultPriority = 200;

  ResizeBehavior() : IBehavior(DefaultPriority) {}

  /** 配置公开供检查；批量修改应调用 SetProperties 以同步 Yoga 约束。 */
  ResizeProperty Properties;
  void SetProperties(const ResizeProperty &properties);
  ResizeRegion HitTest(MouseMovArgs args) const;
  bool IsResizing() const { return ActiveRegion != ResizeRegion::None; }
  bool HasInteractiveGeometry() const { return InteractiveGeometry; }

  /** 输入钩子维护单一边界状态机，光标始终反映捕获时的调整方向。 */
  EventReply OnMouseDown(MouseMovArgs args) override;
  EventReply OnMouseDrag(MouseMovArgs args) override;
  EventReply OnMouseUp(MouseMovArgs args) override;
  void OnPointerCaptureLost() override { ActiveRegion = ResizeRegion::None; }
  MouseCursor GetMouseCursor(MouseMovArgs args) const override;
  bool SetProperty(const std::string &name, const std::string &value) override;

protected:
  void OnAttached() override;

private:
  static MouseCursor CursorForRegion(ResizeRegion region);
  void ApplyMinimumSize() const;

  ResizeRegion ActiveRegion = ResizeRegion::None;
  bool InteractiveGeometry = false;
  float CurrentLeft = 0.0f;
  float CurrentTop = 0.0f;
  float CurrentWidth = 0.0f;
  float CurrentHeight = 0.0f;
};

} // namespace z8::ui
