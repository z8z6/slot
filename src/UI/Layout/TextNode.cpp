#include "UI/Layout/TextNode.h"

#include "UI/Style/Theme.h"
#include "Util/Color.h"

#include <cstdlib>

using namespace z8::ui;

TextNode::TextNode() {
  const auto &style = Theme::Default().Text;
  Color = style.Color;
  // 所有复合控件最终都通过 TextNode 输出文字；从主题复制字体可以同时覆盖
  // Panel、Tree、Menu 和输入框，又允许单个 C++ 节点在必要时显式替换。
  FontFamily = style.FontFamily;
  FontSize = style.FontSize;
  // 文字自身不应像普通容器一样填满父级；默认单行高度由字号留出抗锯齿边缘。
  Style.FlexGrow = 0.0f;
  Style.FlexShrink = 1.0f;
  Style.MinHeight = style.LineHeight;
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
    Style.MinHeight = FontSize * 1.25f;
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
