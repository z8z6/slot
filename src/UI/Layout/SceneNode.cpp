#include "UI/Layout/SceneNode.h"

#include "UI/Style/Theme.h"
#include "yoga/YGNodeStyle.h"

#include <cstdlib>

using namespace z8::ui;

SceneNode::SceneNode() {
  HitTestVisible = true;
  const auto &style = Theme::Default().Panel;
  TitleHeight = style.TitleHeight;
  YGNodeStyleSetFlexDirection(Node, YGFlexDirectionColumn);
  YGNodeStyleSetMargin(Node, YGEdgeAll, 0.0f);
  YGNodeStyleSetPadding(Node, YGEdgeAll, 0.0f);
  YGNodeStyleSetMinWidth(Node, style.MinWidth);
  YGNodeStyleSetMinHeight(Node, style.MinHeight);

  auto titleBar = std::make_unique<RectNode>();
  TitleBarNode = titleBar.get();
  TitleBarNode->Key = "__scene_title_bar";
  TitleBarNode->SetColor(style.TitleColor);
  TitleBarNode->SetBorder(style.BorderColor, style.BorderWidth);
  YGNodeStyleSetMargin(TitleBarNode->Node, YGEdgeAll, 0.0f);
  YGNodeStyleSetPadding(TitleBarNode->Node, YGEdgeAll, 0.0f);
  YGNodeStyleSetHeight(TitleBarNode->Node, TitleHeight);
  YGNodeStyleSetFlexGrow(TitleBarNode->Node, 0.0f);
  YGNodeStyleSetFlexShrink(TitleBarNode->Node, 0.0f);

  auto title = std::make_unique<TextNode>("Viewport");
  TitleNode = title.get();
  TitleNode->Key = "__scene_title";
  TitleNode->Color = style.TitleTextColor;
  TitleNode->Alignment = TextAlignment::Center;
  YGNodeStyleSetMargin(TitleNode->Node, YGEdgeAll, 0.0f);
  YGNodeStyleSetHeight(TitleNode->Node, TitleHeight);
  TitleBarNode->BaseNode::AddChild(std::move(title));
  BaseNode::AddChild(std::move(titleBar));

  auto viewport = std::make_unique<SceneViewportNode>();
  ViewportNode = viewport.get();
  ViewportNode->Key = "__scene_viewport";
  ViewportNode->HitTestVisible = true;
  YGNodeStyleSetMargin(ViewportNode->Node, YGEdgeAll, 0.0f);
  YGNodeStyleSetPadding(ViewportNode->Node, YGEdgeAll, 0.0f);
  YGNodeStyleSetFlexGrow(ViewportNode->Node, 1.0f);
  YGNodeStyleSetFlexShrink(ViewportNode->Node, 1.0f);
  BaseNode::AddChild(std::move(viewport));

  auto *drag = AddBehavior<DragBehavior>();
  drag->SetHandle(TitleBarNode);
  auto *resize = AddBehavior<ResizeBehavior>();
  ResizeProperty resizeProperty;
  resizeProperty.MinWidth = style.MinWidth;
  resizeProperty.MinHeight = style.MinHeight;
  resizeProperty.Border = style.ResizeBorder;
  resize->SetProperties(resizeProperty);
  // 默认占满 DockSpace 剩余区域；拖动后 DockBehavior 会将其切换为浮动视口。
  auto *dock = AddBehavior<DockBehavior>();
  dock->Properties.Placement = DockPlacement::Fill;
}

bool SceneNode::SetProperty(const std::string &name,
                            const std::string &value) {
  if (name == "Title") {
    TitleNode->Text = value;
    return true;
  }
  if (name == "TitleHeight") {
    TitleHeight = (std::max)(1.0f, std::strtof(value.c_str(), nullptr));
    YGNodeStyleSetHeight(TitleBarNode->Node, TitleHeight);
    YGNodeStyleSetHeight(TitleNode->Node, TitleHeight);
    return true;
  }
  return BehaviorNode::SetProperty(name, value);
}

bool SceneNode::HasInteractiveGeometry() const {
  const auto *drag = GetBehavior<DragBehavior>();
  const auto *resize = GetBehavior<ResizeBehavior>();
  return (drag && drag->HasInteractiveGeometry()) ||
         (resize && resize->HasInteractiveGeometry());
}
