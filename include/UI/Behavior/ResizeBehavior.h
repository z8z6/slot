#pragma once

#include "UI/Behavior/UIBehavior.h"
#include "UI/Property/IResizable.h"

namespace z8::ui {

/**
 * 在宿主边界和四角提供尺寸调整的独立手势组件。
 *
 * 组件优先级高于 DragBehavior，保证重叠的标题栏边缘只启动一种几何状态机。
 */
class ResizeBehavior final : public UIBehavior {
public:
  static constexpr int DefaultPriority = 200;

  ResizeBehavior() : UIBehavior(DefaultPriority) {}

  /** SetProperties 同时夹紧配置并同步宿主的 Yoga 最小尺寸约束。 */
  const ResizeProperty &GetProperties() const { return Properties; }
  void SetProperties(const ResizeProperty &properties);
  ResizeRegion HitTest(MouseMovArgs args) const;
  bool IsResizing() const { return ActiveRegion != ResizeRegion::None; }
  bool HasInteractiveGeometry() const { return InteractiveGeometry; }

  /** 输入钩子维护单一边界状态机，光标始终反映捕获时的调整方向。 */
  UIEventReply OnMouseDown(MouseMovArgs args) override;
  bool OnMouseDrag(MouseMovArgs args) override;
  bool OnMouseUp(MouseMovArgs args) override;
  void OnCaptureLost() override { ActiveRegion = ResizeRegion::None; }
  MouseCursor GetMouseCursor(MouseMovArgs args) const override;
  bool SetProperty(const std::string &name, const std::string &value) override;

protected:
  void OnAttached() override;

private:
  static MouseCursor CursorForRegion(ResizeRegion region);
  void ApplyMinimumSize() const;

  ResizeProperty Properties;
  ResizeRegion ActiveRegion = ResizeRegion::None;
  bool InteractiveGeometry = false;
  float CurrentLeft = 0.0f;
  float CurrentTop = 0.0f;
  float CurrentWidth = 0.0f;
  float CurrentHeight = 0.0f;
};

} // namespace z8::ui
