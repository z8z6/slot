#include "UI/Property/IScrollable.h"

#include <algorithm>

using namespace z8::ui;

void IScrollable::SetScrollProperties(const ScrollProperty& properties) {
  ScrollProperties = properties;
  SetScrollOffsetY(ScrollOffsetY);
}

void IScrollable::SetVerticalScrollRange(float viewportExtent,
                                         float contentExtent) {
  MaximumScrollOffsetY = ScrollProperties.Enabled && ScrollProperties.Vertical
      ? (std::max)(0.0f, contentExtent - viewportExtent)
      : 0.0f;
  SetScrollOffsetY(ScrollOffsetY);
}

void IScrollable::SetScrollOffsetY(float offset) {
  const float next = std::clamp(offset, 0.0f, MaximumScrollOffsetY);
  if (next == ScrollOffsetY) {
    // range 或复合控件可能刚变化，即使数值相同也要重新同步视觉结构。
    ScrollOffsetChanged(next);
    return;
  }
  ScrollOffsetY = next;
  ScrollOffsetChanged(ScrollOffsetY);
}

bool IScrollable::ScrollVerticalBy(float delta) {
  if (!ScrollProperties.Enabled || !ScrollProperties.Vertical ||
      MaximumScrollOffsetY <= 0.0f)
    return false;
  SetScrollOffsetY(ScrollOffsetY + delta);
  return true;
}
