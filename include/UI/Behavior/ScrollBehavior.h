#pragma once

#include "UI/Behavior/IBehavior.h"

namespace z8::ui {

class BaseNode;
class ScrollBarNode;

/** 滚动条可见性策略；Auto 仅在对应方向内容溢出时显示。 */
enum class ScrollBarVisibility {
  Hidden,
  Auto,
  Visible
};

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
 * 连接 viewport、content 与可选滚动条的滚动行为。
 *
 * 三个节点均为非拥有观察指针，并应与行为宿主处于同一复合控件子树。行为只
 * 依赖 BaseNode 的裁剪和 ScrollBarNode 的 value/range
 */
class ScrollBehavior final : public IBehavior {
  BaseNode *Viewport = nullptr;
  BaseNode *Content = nullptr;
  ScrollBarNode *VerticalScrollBar = nullptr;
  float OffsetY = 0.0f;
  float MaximumOffsetY = 0.0f;
public:
  ScrollProperty Properties;
  ScrollBehavior() = default;

  void BindVertical(BaseNode *viewport, BaseNode *content, ScrollBarNode *scrollBar);
  void SetProperties(const ScrollProperty &properties);
  bool SetProperty(const std::string &name, const std::string &value) override;
  void SetOffsetY(float offset);
  float GetOffsetY() const { return OffsetY; }
  float GetMaximumOffsetY() const { return MaximumOffsetY; }
;
  EventReply OnMouseWheel(MouseWheelArgs args) override;
  void OnAfterLayout() override;
  void OnDetached() override;

private:
  void ApplyProperties();
  void SynchronizeVisuals() const;
  void UpdateRange();
};

} // namespace z8::ui
