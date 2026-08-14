#pragma once

#include "UI/Layout/RectNode.h"
#include "UI/Layout/TextNode.h"

#include <functional>

namespace z8::ui {

/** 带标签的布尔开关；状态归控件所有，外部通过回调或 ConsumeChanged 同步。 */
class ToggleNode final : public RectNode {
private:
  bool Armed = false;
  bool ChangePending = false;

public:
  RectNode *IndicatorNode = nullptr;
  TextNode *LabelNode = nullptr;
  bool Checked = false;
  bool Enabled = true;
  std::function<void(bool)> ValueChanged;

  ToggleNode();

  bool ConsumeChanged();
  const char *TypeName() const override { return "Toggle"; }
  EventReply OnKeyDown(KeyArgs args) override;
  EventReply OnMouseDown(MouseMovArgs args) override;
  EventReply OnMouseUp(MouseMovArgs args) override;
  void OnPointerCaptureLost() override { Armed = false; }
  bool SetChecked(bool checked, bool notify = true);
  void SetEnabled(bool enabled);
  bool SetProperty(const std::string &name, const std::string &value) override;
  void SetText(const std::string &text);

private:
  void OnVisualStateChanged() override;
};

} // namespace z8::ui
