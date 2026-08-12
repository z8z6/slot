#include "UI/Layout/ScrollNode.h"

#include "UI/Style/UITheme.h"
#include "yoga/YGNodeStyle.h"

using namespace z8::ui;

ScrollNode::ScrollNode() {
  const auto &style = UITheme::Modern().Panel;
  // ScrollNode 没有自己的矩形视觉，但空白 viewport 仍要接收滚轮输入。
  HitTestVisible = true;

  auto viewport = std::make_unique<BaseNode>();
  ViewportNode = viewport.get();
  ViewportNode->Key = "__viewport";
  YGNodeStyleSetFlexGrow(ViewportNode->Node, 1.0f);
  YGNodeStyleSetFlexShrink(ViewportNode->Node, 1.0f);

  auto content = std::make_unique<BaseNode>();
  ContentNode = content.get();
  ContentNode->Key = "__content";
  YGNodeStyleSetFlexGrow(ContentNode->Node, 0.0f);
  YGNodeStyleSetFlexShrink(ContentNode->Node, 0.0f);
  YGNodeStyleSetWidthPercent(ContentNode->Node, 100.0f);
  YGNodeStyleSetPadding(ContentNode->Node, YGEdgeAll, style.ContentPadding);
  ViewportNode->AddChild(std::move(content));
  BaseNode::AddChild(std::move(viewport));

  auto scrollBar =
      std::make_unique<ScrollBarNode>(ScrollBarOrientation::Vertical);
  VerticalScrollBarNode = scrollBar.get();
  VerticalScrollThumbNode = scrollBar->ThumbNode;
  VerticalScrollBarNode->Key = "__vertical_scrollbar";
  YGNodeStyleSetPositionType(VerticalScrollBarNode->Node,
                             YGPositionTypeAbsolute);
  YGNodeStyleSetWidth(VerticalScrollBarNode->Node, style.ScrollBarThickness);
  BaseNode::AddChild(std::move(scrollBar));
  SetVerticalBarInsets(2.0f, style.ResizeBorder + 2.0f, 2.0f);

  AddBehavior<ScrollBehavior>()->BindVertical(
      ViewportNode, ContentNode, VerticalScrollBarNode);
}

void ScrollNode::SetVerticalBarInsets(float top, float right, float bottom) {
  YGNodeStyleSetPosition(VerticalScrollBarNode->Node, YGEdgeTop, top);
  YGNodeStyleSetPosition(VerticalScrollBarNode->Node, YGEdgeRight, right);
  YGNodeStyleSetPosition(VerticalScrollBarNode->Node, YGEdgeBottom, bottom);
}
