#pragma once

#include "UI/Layout/RectNode.h"
#include "UI/Layout/TextNode.h"

#include <functional>

namespace z8::ui {

/** 紧凑按钮控件；拥有标签、主题状态和一次完整点击手势。 */
class ButtonNode : public RectNode {
private:
  bool Armed = false;
  bool ClickPending = false;

public:
  TextNode *LabelNode = nullptr;
  bool Enabled = true;
  std::function<void()> Clicked;

  ButtonNode();

  bool ConsumeClicked();
  const char *TypeName() const override { return "Button"; }
  EventReply OnKeyDown(KeyArgs args) override;
  EventReply OnMouseDown(MouseMovArgs args) override;
  EventReply OnMouseUp(MouseMovArgs args) override;
  void OnPointerCaptureLost() override { Armed = false; }
  bool SetProperty(const std::string &name, const std::string &value) override;
  void SetEnabled(bool enabled);
  void SetText(const std::string &text);

protected:
  void Activate();
  void OnVisualStateChanged() override;
};

} // namespace z8::ui
