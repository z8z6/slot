#pragma once

#include "UI/Layout/RectNode.h"

#include <functional>

namespace z8::ui {

/** 单轴数值 Slider；布局只定义命中轨道，动态 Fill/Thumb 在同步阶段求解。 */
class SliderNode final : public RectNode {
private:
  bool ChangePending = false;
  bool Dragging = false;

public:
  RectNode *TrackNode = nullptr;
  RectNode *FillNode = nullptr;
  RectNode *ThumbNode = nullptr;
  float Minimum = 0.0f;
  float Maximum = 1.0f;
  float Value = 0.0f;
  float Step = 0.0f;
  bool Enabled = true;
  std::function<void(float)> ValueChanged;

  SliderNode();

  bool ConsumeChanged();
  const char *TypeName() const override { return "Slider"; }
  EventReply OnKeyDown(KeyArgs args) override;
  EventReply OnMouseDown(MouseMovArgs args) override;
  EventReply OnMouseDrag(MouseMovArgs args) override;
  EventReply OnMouseUp(MouseMovArgs args) override;
  void OnPointerCaptureLost() override { Dragging = false; }
  void SetEnabled(bool enabled);
  bool SetProperty(const std::string &name, const std::string &value) override;
  bool SetRange(float minimum, float maximum);
  bool SetValue(float value, bool notify = true);
  void Synchronize() override;

private:
  void OnVisualStateChanged() override;
  void UpdateFromPointer(float clientX);
};

} // namespace z8::ui
