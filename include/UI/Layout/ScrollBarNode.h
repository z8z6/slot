#pragma once

#include "UI/Layout/RectNode.h"

#include <functional>

namespace z8::ui {

/** 滚动条方向；当前 Panel 使用 Vertical，接口预留水平 ScrollView。 */
enum class UIScrollBarOrientation { Horizontal, Vertical };

/**
 * 独立滚动条复合控件。
 *
 * 它只负责 value/range、轨道分页和滑块捕获，不拥有被滚动内容；父级 ScrollView
 * 通过 ValueChanged 同步内容偏移，避免滚动条依赖 Panel 的内部结构。
 */
class ScrollBarNode : public RectNode {
public:
  RectNode* ThumbNode;
  UIScrollBarOrientation Orientation;
  std::function<void(float)> ValueChanged;

  explicit ScrollBarNode(
      UIScrollBarOrientation orientation = UIScrollBarOrientation::Vertical);

  const char* TypeName() const override { return "ScrollBar"; }
  bool OnMouseDown(MouseMovArgs args) override;
  bool OnMouseDrag(MouseMovArgs args) override;
  bool OnMouseUp(MouseMovArgs args) override;
  void OnLayoutUpdated() override;

  void SetMetrics(float viewportExtent, float contentExtent);
  void SetValue(float value, bool notify = true);
  float GetValue() const { return Value; }
  float GetMaximum() const { return Maximum; }

private:
  float ViewportExtent = 0.0f;
  float ContentExtent = 0.0f;
  float Value = 0.0f;
  float Maximum = 0.0f;
  float DragScale = 0.0f;
  bool DraggingThumb = false;
};

} // namespace z8::ui
