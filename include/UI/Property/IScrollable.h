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
 * 可滚动能力的兼容查询接口。
 *
 * offset/range 及视觉同步已经移入 ScrollBehavior；接口自身不再要求派生控件
 * 实现回调，因此滚动能力可以独立挂载到其他复合控件。
 */
class IScrollable : public virtual IProperty {
public:
  ~IScrollable() override = default;
  virtual const ScrollProperty &GetScrollProperties() const = 0;
  virtual void SetScrollProperties(const ScrollProperty &properties) = 0;
  virtual float GetScrollOffsetY() const = 0;
  virtual float GetMaximumScrollOffsetY() const = 0;
  virtual void SetScrollOffsetY(float offset) = 0;
};

} // namespace z8::ui
