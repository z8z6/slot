//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/RectNode.h"

#include "Object/BuiltinObject.h"
#include "UI/Style/Theme.h"

using namespace z8::ui;

RectNode::RectNode() : DrawNode(std::make_unique<RectUIObject>()) {
  const auto &style = Theme::Default().Rect;
  SetColor(style.Color);
  SetBorder(style.BorderColor, style.BorderWidth);
  SetCornerRadius(style.CornerRadius);
  Style.Margin = style.Margin;
  Style.Padding = style.Padding;
  Style.MinWidth = style.MinWidth;
  Style.MinHeight = style.MinHeight;
}
