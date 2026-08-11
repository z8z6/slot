#pragma once

#include "UI/Property/IProperty.h"

namespace z8::ui {

/** 滚动条可见性策略；Auto 仅在对应方向内容溢出时显示。 */
enum class ScrollBarVisibility { Hidden, Auto, Visible };

/** 滚动方向、滚动条策略及鼠标滚轮步长。 */
struct ScrollProperty {
  bool Enabled = true;
  bool Horizontal = false;
  bool Vertical = true;
  ScrollBarVisibility HorizontalScrollBar = ScrollBarVisibility::Hidden;
  ScrollBarVisibility VerticalScrollBar = ScrollBarVisibility::Auto;
  float WheelStep = 40.0f;
};

/**
 * 可滚动能力 mixin。
 *
 * 它拥有 offset/range 的状态不变量，但不知道 viewport、content 或滚动条的
 * 控件类型；派生控件通过 ScrollOffsetChanged 将偏移应用到自己的复合结构。
 */
class IScrollable : public virtual IProperty {
public:
  virtual const ScrollProperty& GetScrollProperties() const {
    return ScrollProperties;
  }
  virtual void SetScrollProperties(const ScrollProperty& properties);
  virtual float GetScrollOffsetY() const { return ScrollOffsetY; }
  virtual float GetMaximumScrollOffsetY() const {
    return MaximumScrollOffsetY;
  }
  virtual void SetScrollOffsetY(float offset);

protected:
  void SetVerticalScrollRange(float viewportExtent, float contentExtent);
  bool ScrollVerticalBy(float delta);
  virtual void ScrollOffsetChanged(float offset) = 0;

private:
  ScrollProperty ScrollProperties;
  float ScrollOffsetY = 0.0f;
  float MaximumScrollOffsetY = 0.0f;
};

} // namespace z8::ui
