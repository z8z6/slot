#include "UI/Layout/SceneNode.h"

#include "UI/Style/Theme.h"

#include <cstdlib>

using namespace z8::ui;

SceneNode::SceneNode() {
  HitTestVisible = true;
  const auto &style = Theme::Default().Panel;
  TitleHeight = style.TitleHeight;
  Style.Direction = FlexDirection::Column;
  Style.Margin = 0.0f;
  Style.Padding = 0.0f;
  Style.MinWidth = style.MinWidth;
  Style.MinHeight = style.MinHeight;

  auto titleBar = std::make_unique<RectNode>();
  TitleBarNode = titleBar.get();
  TitleBarNode->Key = "__scene_title_bar";
  TitleBarNode->SetColor(style.TitleColor);
  TitleBarNode->SetBorder(style.BorderColor, style.BorderWidth);
  TitleBarNode->Style.Margin = 0.0f;
  TitleBarNode->Style.Padding = 0.0f;
  TitleBarNode->Style.Height = TitleHeight;
  TitleBarNode->Style.FlexGrow = 0.0f;
  TitleBarNode->Style.FlexShrink = 0.0f;

  auto title = std::make_unique<TextNode>("Viewport");
  TitleNode = title.get();
  TitleNode->Key = "__scene_title";
  TitleNode->Color = style.TitleTextColor;
  TitleNode->Alignment = TextAlignment::Leading;
  TitleNode->Style.Margin = SpacingStyle::Medium;
  TitleNode->Style.Height = TitleHeight;
  TitleBarNode->BaseNode::AddChild(std::move(title));
  BaseNode::AddChild(std::move(titleBar));

  auto viewport = std::make_unique<SceneViewportNode>();
  ViewportNode = viewport.get();
  ViewportNode->Key = "__scene_viewport";
  ViewportNode->HitTestVisible = true;
  ViewportNode->Style.Margin = 0.0f;
  ViewportNode->Style.Padding = 0.0f;
  ViewportNode->Style.FlexGrow = 1.0f;
  ViewportNode->Style.FlexShrink = 1.0f;
  BaseNode::AddChild(std::move(viewport));

  auto *drag = AddBehavior<DragBehavior>();
  drag->SetHandle(TitleBarNode);
  auto *resize = AddBehavior<ResizeBehavior>();
  ResizeProperty resizeProperty;
  resizeProperty.MinWidth = style.MinWidth;
  resizeProperty.MinHeight = style.MinHeight;
  resizeProperty.Border = style.ResizeBorder;
  resize->SetProperties(resizeProperty);
  // 默认占满 DockSpace 剩余区域；标题拖动只用于重新选择停靠槽。
  auto *dock = AddBehavior<DockBehavior>();
  dock->Properties.Placement = DockPlacement::Fill;
}

bool SceneNode::SetProperty(const std::string &name, const std::string &value) {
  if (name == "Title") {
    TitleNode->Text = value;
    return true;
  }
  if (name == "TitleHeight") {
    TitleHeight = (std::max)(1.0f, std::strtof(value.c_str(), nullptr));
    TitleBarNode->Style.Height = TitleHeight;
    TitleNode->Style.Height = TitleHeight;
    return true;
  }
  return BehaviorNode::SetProperty(name, value);
}

bool SceneNode::HasInteractiveGeometry() const {
  const auto *resize = GetBehavior<ResizeBehavior>();
  return resize && resize->HasInteractiveGeometry();
}
