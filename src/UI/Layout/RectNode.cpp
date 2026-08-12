//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/RectNode.h"

#include "Object/UIObject/RectUIObject.h"
#include "UI/Style/Theme.h"
#include "yoga/YGNodeStyle.h"

using namespace z8::ui;

RectNode::RectNode() : DrawNode(std::make_unique<RectUIObject>()) {
  const auto &style = Theme::Default().Rect;
  SetColor(style.Color);
  YGNodeStyleSetMargin(Node, YGEdgeAll, style.Margin);
  YGNodeStyleSetPadding(Node, YGEdgeAll, style.Padding);
  YGNodeStyleSetMinWidth(Node, style.MinWidth);
  YGNodeStyleSetMinHeight(Node, style.MinHeight);
}
