#include "UI/Layout/ToolBarNode.h"

#include "UI/Behavior/DockBehavior.h"
#include "UI/Layout/MenuNode.h"
#include "UI/Style/Theme.h"

using namespace z8::ui;

ToolBarNode::ToolBarNode() {
  const auto &style = Theme::Default().ToolBar;
  Style.Direction = FlexDirection::Row;
  Style.Height = style.Height;
  Style.MinWidth = 0.0f;
  Style.MinHeight = style.Height;
  Style.Margin = 0.0f;
  Style.Padding = style.Padding;
  Style.FlexGrow = 0.0f;
  Style.FlexShrink = 0.0f;
  SetColor(style.Color);
  SetBorder(style.BorderColor, style.BorderWidth);
  SetCornerRadius(0.0f);

  // ToolBar 直接占据 DockTree 的 Top leaf；它没有 Panel 标题、页签和拖拽
  // 行为，因此不会被 NormalizePanelGroups 包装。
  auto *dock = AddBehavior<DockBehavior>();
  dock->Properties.Placement = DockPlacement::Top;
  dock->Properties.Extent = style.Height;
  // Menu Bar 是窗口框架的一部分而不是可调工作区：固定主题高度，并让
  // DockTree 不为它生成可命中的共享分割线。
  dock->Properties.Resizable = false;
}

BaseNode *ToolBarNode::AddChild(std::unique_ptr<BaseNode> child) {
  if (auto *menu = dynamic_cast<MenuNode *>(child.get()))
    menu->SetTopLevel(true);
  return BaseNode::AddChild(std::move(child));
}
