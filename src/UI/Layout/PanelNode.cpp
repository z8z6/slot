//
// Created by zhou_zhengming on 2026/7/31.
//

#include "UI/Layout/PanelNode.h"

#include "Object/UIObject/RectUIObject.h"
#include "UI/Style/UITheme.h"
#include "yoga/YGNodeStyle.h"

#include <cstdlib>

using namespace z8::ui;

PanelNode::PanelNode()
    : TitleNode(nullptr), ScrollViewportNode(nullptr), ContentNode(nullptr),
      VerticalScrollBarNode(nullptr), VerticalScrollThumbNode(nullptr) {
  const auto &style = UITheme::Modern().Panel;
  // Panel 自身纵向排列；标题栏固定高度，内容宿主占据剩余空间。
  YGNodeStyleSetFlexDirection(Node, YGFlexDirectionColumn);
  SetColor(style.Color);
  YGNodeStyleSetMargin(Node, YGEdgeAll, style.Margin);
  YGNodeStyleSetPadding(Node, YGEdgeAll, style.Padding);
  YGNodeStyleSetMinWidth(Node, style.MinimumWidth);
  YGNodeStyleSetMinHeight(Node, style.MinimumHeight);
  TitleHeight = style.TitleHeight;

  auto title = std::make_unique<RectNode>();
  TitleNode = title.get();
  TitleNode->Key = "__title";
  TitleNode->SetColor(style.TitleColor);
  // 内部标题不是普通列表项，清除 Rect 的外边距以贴合 Panel 边缘。
  YGNodeStyleSetMargin(TitleNode->Node, YGEdgeAll, 0.0f);
  YGNodeStyleSetHeight(TitleNode->Node, TitleHeight);
  YGNodeStyleSetFlexGrow(TitleNode->Node, 0.0f);
  YGNodeStyleSetFlexShrink(TitleNode->Node, 0.0f);
  BaseNode::AddChild(std::move(title));

  auto viewport = std::make_unique<BaseNode>();
  ScrollViewportNode = viewport.get();
  ScrollViewportNode->Key = "__scroll_viewport";
  YGNodeStyleSetFlexGrow(ScrollViewportNode->Node, 1.0f);
  YGNodeStyleSetFlexShrink(ScrollViewportNode->Node, 1.0f);
  // 裁剪子元素
  ScrollViewportNode->ClipChildren = true;

  auto content = std::make_unique<BaseNode>();
  ContentNode = content.get();
  ContentNode->Key = "__content";
  // Content 按内容自然尺寸增长，Viewport 单独负责约束与裁剪。
  YGNodeStyleSetFlexGrow(ContentNode->Node, 0.0f);
  YGNodeStyleSetFlexShrink(ContentNode->Node, 0.0f);
  YGNodeStyleSetWidthPercent(ContentNode->Node, 100.0f);
  YGNodeStyleSetPadding(ContentNode->Node, YGEdgeAll, style.ContentPadding);
  ScrollViewportNode->AddChild(std::move(content));
  BaseNode::AddChild(std::move(viewport));

  auto scrollBar =
      std::make_unique<ScrollBarNode>(ScrollBarOrientation::Vertical);
  VerticalScrollBarNode = scrollBar.get();
  VerticalScrollBarNode->Key = "__vertical_scrollbar";
  YGNodeStyleSetPositionType(VerticalScrollBarNode->Node,
                             YGPositionTypeAbsolute);
  YGNodeStyleSetPosition(VerticalScrollBarNode->Node, YGEdgeTop,
                         TitleHeight + 2.0f);
  YGNodeStyleSetPosition(VerticalScrollBarNode->Node, YGEdgeRight,
                         style.ResizeBorder + 2.0f);
  YGNodeStyleSetPosition(VerticalScrollBarNode->Node, YGEdgeBottom, 2.0f);
  YGNodeStyleSetWidth(VerticalScrollBarNode->Node, style.ScrollBarThickness);
  VerticalScrollThumbNode = VerticalScrollBarNode->ThumbNode;
  BaseNode::AddChild(std::move(scrollBar));

  // Panel 只组装视觉和能力。行为优先级由组件自身声明，因此边缘 Resize 会在
  // 标题 Drag 之前获得同一次按下，而 Panel 不需要知道仲裁细节。
  auto *drag = AddBehavior<DragBehavior>();
  drag->SetHandle(TitleNode);
  auto *resizeBehavior = AddBehavior<ResizeBehavior>();
  ResizeProperty resize;
  resize.MinWidth = style.MinimumWidth;
  resize.MinHeight = style.MinimumHeight;
  resize.Border = style.ResizeBorder;
  resizeBehavior->SetProperties(resize);
  auto *scroll = AddBehavior<ScrollBehavior>();
  scroll->BindVertical(ScrollViewportNode, ContentNode, VerticalScrollBarNode);
  // DockBehavior 在 DragBehavior 之后挂载，以观察完整手势并保持能力可替换。
  AddBehavior<DockBehavior>();
}

BaseNode *PanelNode::ContentHost() { return ContentNode; }

bool PanelNode::SetProperty(const std::string &name, const std::string &value) {
  if (name == "Title") {
    // 当前渲染器尚无文字栅格化，先保留标题语义；标题栏几何已经可见。
    Title = value;
    return true;
  }
  if (name == "TitleHeight") {
    TitleHeight = std::strtof(value.c_str(), nullptr);
    YGNodeStyleSetHeight(TitleNode->Node, TitleHeight);
    YGNodeStyleSetPosition(VerticalScrollBarNode->Node, YGEdgeTop,
                           TitleHeight + 2.0f);
    return true;
  }
  if (name == "TitleColor") {
    DirectX::XMFLOAT4 color;
    if (!ParseUIColor(value, color))
      return false;
    return TitleNode->SetColor(color);
  }
  // ResizeBorder 同时影响行为命中宽度与滚动条避让距离；这是 Panel 视觉组装
  // 唯一需要消费的能力属性，其余属性由 BaseNode 自动转发给相应 Behavior。
  if (name == "ResizeBorder") {
    auto *resize = GetBehavior<ResizeBehavior>();
    if (!resize || !resize->SetProperty(name, value))
      return false;
    YGNodeStyleSetPosition(VerticalScrollBarNode->Node, YGEdgeRight,
                           resize->Properties.Border + 2.0f);
    return true;
  }
  return RectNode::SetProperty(name, value);
}
