#include "UI/Layout/TextNode.h"

#include "UI/Style/UITheme.h"
#include "yoga/YGNodeStyle.h"

#include <cstdlib>

using namespace z8::ui;

TextNode::TextNode() {
  // 文字自身不应像普通容器一样填满父级；默认单行高度由字号留出抗锯齿边缘。
  YGNodeStyleSetFlexGrow(Node, 0.0f);
  YGNodeStyleSetFlexShrink(Node, 1.0f);
  YGNodeStyleSetMinHeight(Node, FontSize * 1.25f);
}

TextNode::TextNode(std::string text) : TextNode() { Text = std::move(text); }

bool TextNode::SetProperty(const std::string &name,
                           const std::string &value) {
  if (name == "Text") {
    Text = value;
    return true;
  }
  if (name == "FontSize") {
    FontSize = (std::max)(1.0f, std::strtof(value.c_str(), nullptr));
    YGNodeStyleSetMinHeight(Node, FontSize * 1.25f);
    return true;
  }
  if (name == "TextColor")
    return ParseUIColor(value, Color);
  if (name == "TextAlignment") {
    if (value == "Leading")
      Alignment = TextAlignment::Leading;
    else if (value == "Center")
      Alignment = TextAlignment::Center;
    else if (value == "Trailing")
      Alignment = TextAlignment::Trailing;
    else
      return false;
    return true;
  }
  if (name == "TextWrap") {
    if (value == "true" || value == "True" || value == "1")
      Wrap = true;
    else if (value == "false" || value == "False" || value == "0")
      Wrap = false;
    else
      return false;
    return true;
  }
  return BaseNode::SetProperty(name, value);
}
