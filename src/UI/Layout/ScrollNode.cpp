#include "UI/Layout/ScrollNode.h"

#include "UI/Style/Theme.h"

using namespace z8::ui;

ScrollNode::ScrollNode() {
  const auto &panelStyle = Theme::Default().Panel;
  const auto &scrollBarStyle = Theme::Default().ScrollBar;
  // ScrollNode 没有自己的矩形视觉，但空白 viewport 仍要接收滚轮输入。
  HitTestVisible = true;

  auto viewport = std::make_unique<BaseNode>();
  ViewportNode = viewport.get();
  ViewportNode->Key = "__viewport";
  ViewportNode->Style.FlexGrow = 1.0f;
  ViewportNode->Style.FlexShrink = 1.0f;

  auto content = std::make_unique<BaseNode>();
  ContentNode = content.get();
  ContentNode->Key = "__content";
  ContentNode->Style.FlexGrow = 0.0f;
  ContentNode->Style.FlexShrink = 0.0f;
  ContentNode->Style.WidthPercent = 100.0f;
  ContentNode->Style.Padding = panelStyle.ContentPadding;
  ViewportNode->AddChild(std::move(content));
  BaseNode::AddChild(std::move(viewport));

  auto scrollBar =
      std::make_unique<ScrollBarNode>(ScrollBarOrientation::Vertical);
  VerticalScrollBarNode = scrollBar.get();
  VerticalScrollThumbNode = scrollBar->ThumbNode;
  VerticalScrollBarNode->Key = "__vertical_scrollbar";
  VerticalScrollBarNode->Style.Position = PositionType::Absolute;
  VerticalScrollBarNode->Style.Width = scrollBarStyle.ScrollBarThickness;
  BaseNode::AddChild(std::move(scrollBar));
  // 轨道贴齐 ScrollNode 右边界；ResizeBehavior 只负责命中，不应侵占内容视觉
  // 空间，否则加宽滚动条后右侧仍会留下明显的空槽。
  SetVerticalBarInsets(2.0f, 0.0f, 2.0f);

  AddBehavior<ScrollBehavior>()->BindVertical(
      ViewportNode, ContentNode, VerticalScrollBarNode);
}

void ScrollNode::SetVerticalBarInsets(float top, float right, float bottom) {
  VerticalScrollBarNode->Style.Top = top;
  VerticalScrollBarNode->Style.Right = right;
  VerticalScrollBarNode->Style.Bottom = bottom;
}
