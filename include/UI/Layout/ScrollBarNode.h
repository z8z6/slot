#pragma once

#include "UI/Layout/RectNode.h"

#include <functional>

namespace z8::ui {

/** 滚动条方向；当前 Panel 使用 Vertical，接口预留水平 ScrollView。 */
enum class ScrollBarOrientation { Horizontal, Vertical };

/**
 * 独立滚动条复合控件。
 *
 * 它只负责 value/range、轨道分页和滑块捕获，不拥有被滚动内容；父级 ScrollView
 * 通过 ValueChanged 同步内容偏移，避免滚动条依赖 Panel 的内部结构。
 */
class ScrollBarNode : public RectNode {
private:
  float ViewportExtent = 0.0f;
  float ContentExtent = 0.0f;
  float Value = 0.0f;
  float Maximum = 0.0f;
  float DragScale = 0.0f;
  bool DraggingThumb = false;

public:
  RectNode *ThumbNode;
  ScrollBarOrientation Orientation;
  std::function<void(float)> ValueChanged;

  explicit ScrollBarNode(
      ScrollBarOrientation orientation = ScrollBarOrientation::Vertical);

  const char *TypeName() const override { return "ScrollBar"; }
  EventReply OnMouseDown(MouseMovArgs args) override;
  EventReply OnMouseDrag(MouseMovArgs args) override;
  EventReply OnMouseUp(MouseMovArgs args) override;
  void OnPointerCaptureLost() override { DraggingThumb = false; }
  void OnAfterLayout() override;

  /** 更新 viewport/content 比例，并据此计算滑块长度和最大值。 */
  void SetMetrics(float viewportExtent, float contentExtent);
  /** 设置经过夹紧的滚动值；notify 控制是否向 ScrollBehavior 回传。 */
  void SetValue(float value, bool notify = true);

private:
  void OnVisualStateChanged() override;
};

} // namespace z8::ui
