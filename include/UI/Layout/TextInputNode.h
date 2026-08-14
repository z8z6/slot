#pragma once

#include "UI/Layout/RectNode.h"
#include "UI/Layout/TextNode.h"

#include <functional>
#include <string>

namespace z8::ui {

/** 单行 UTF-8 文本输入；字符由 WM_CHAR 通道输入，导航和编辑键由 KeyDown 处理。
 */
class TextInputNode final : public RectNode {
private:
  size_t CaretOffset = 0;
  bool ChangePending = false;

public:
  TextNode *DisplayNode = nullptr;
  RectNode *CaretNode = nullptr;
  std::string Text;
  std::string Placeholder;
  bool Enabled = true;
  std::function<void(const std::string &)> TextChanged;
  std::function<void(const std::string &)> Submitted;

  TextInputNode();

  bool ConsumeChanged();
  const char *TypeName() const override { return "TextInput"; }
  EventReply OnKeyDown(KeyArgs args) override;
  EventReply OnMouseDown(MouseMovArgs args) override;
  EventReply OnMouseUp(MouseMovArgs args) override;
  EventReply OnTextInput(wchar_t character) override;
  void SetEnabled(bool enabled);
  void SetPlaceholder(const std::string &placeholder);
  bool SetProperty(const std::string &name, const std::string &value) override;
  bool SetText(const std::string &text, bool notify = true);
  void Synchronize() override;

private:
  void NotifyChanged();
  void OnVisualStateChanged() override;
  void RefreshTextVisual();
};

} // namespace z8::ui
