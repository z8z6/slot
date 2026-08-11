//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/RectNode.h"

#include "Object/UIObject/RectUIObject.h"
#include "UI/Style/UITheme.h"
#include "yoga/YGNodeStyle.h"

using namespace z8::ui;

RectNode::RectNode() {
  UO = std::make_unique<RectUIObject>();
  const auto& style = UITheme::Modern().Rect;
  SetColor(style.Color);
  YGNodeStyleSetMargin(GetYogaNode(), YGEdgeAll, style.Margin);
  YGNodeStyleSetPadding(GetYogaNode(), YGEdgeAll, style.Padding);
  YGNodeStyleSetMinWidth(GetYogaNode(), style.MinimumWidth);
  YGNodeStyleSetMinHeight(GetYogaNode(), style.MinimumHeight);
}
