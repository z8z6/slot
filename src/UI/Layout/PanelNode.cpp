//
// Created by zhou_zhengming on 2026/7/31.
//

#include "UI/Layout/PanelNode.h"

#include "UI/Style/Theme.h"
#include "Util/Color.h"
#include "yoga/YGNodeStyle.h"

#include <cstdlib>

using namespace z8::ui;

PanelNode::PanelNode()
    : TitleNode(nullptr), ScrollAreaNode(nullptr) {
  const auto &style = Theme::Default().Panel;
  // Panel 自身纵向排列；标题栏固定高度，内容宿主占据剩余空间。
  YGNodeStyleSetFlexDirection(Node, YGFlexDirectionColumn);
  SetColor(style.Color);
  SetBorder(style.BorderColor, style.BorderWidth);
  YGNodeStyleSetMargin(Node, YGEdgeAll, style.Margin);
  YGNodeStyleSetPadding(Node, YGEdgeAll, style.Padding);
  YGNodeStyleSetMinWidth(Node, style.MinWidth);
  YGNodeStyleSetMinHeight(Node, style.MinHeight);
  TitleHeight = style.TitleHeight;

  auto title = std::make_unique<TextNode>();
  TitleNode = title.get();
  TitleNode->Key = "__title";
  TitleNode->Color = style.TitleTextColor;
  TitleNode->Alignment = TextAlignment::Center;
  // 标题文字独占标题栏布局框；背景仍由 Panel 绘制，避免为标题额外创建矩形。
  YGNodeStyleSetMargin(TitleNode->Node, YGEdgeAll, 0.0f);
  YGNodeStyleSetHeight(TitleNode->Node, TitleHeight);
  YGNodeStyleSetFlexGrow(TitleNode->Node, 0.0f);
  YGNodeStyleSetFlexShrink(TitleNode->Node, 0.0f);
  BaseNode::AddChild(std::move(title));

  auto scroll = std::make_unique<z8::ui::ScrollNode>();
  ScrollAreaNode = scroll.get();
  ScrollAreaNode->Key = "__scroll";
  YGNodeStyleSetFlexGrow(ScrollAreaNode->Node, 1.0f);
  YGNodeStyleSetFlexShrink(ScrollAreaNode->Node, 1.0f);
  ScrollAreaNode->SetVerticalBarInsets(2.0f, style.ResizeBorder + 2.0f,
                                       2.0f);
  BaseNode::AddChild(std::move(scroll));

  // Panel 只组装视觉和能力。行为优先级由组件自身声明，因此边缘 Resize 会在
  // 标题 Drag 之前获得同一次按下，而 Panel 不需要知道仲裁细节。
  auto *drag = AddBehavior<DragBehavior>();
  drag->SetHandle(TitleNode);
  auto *resizeBehavior = AddBehavior<ResizeBehavior>();
  ResizeProperty resize;
  resize.MinWidth = style.MinWidth;
  resize.MinHeight = style.MinHeight;
  resize.Border = style.ResizeBorder;
  resizeBehavior->SetProperties(resize);
  // DockBehavior 在 DragBehavior 之后挂载，以观察完整手势并保持能力可替换。
  AddBehavior<DockBehavior>();
}

BaseNode *PanelNode::ContentHost() { return ScrollAreaNode->ContentHost(); }

bool PanelNode::SetProperty(const std::string &name, const std::string &value) {
  if (name == "Title") {
    TitleNode->Text = value;
    return true;
  }
  if (name == "TitleHeight") {
    TitleHeight = std::strtof(value.c_str(), nullptr);
    YGNodeStyleSetHeight(TitleNode->Node, TitleHeight);
    return true;
  }
  if (name == "TitleColor") {
    DirectX::XMFLOAT4 color;
    if (!ParseUIColor(value, color))
      return false;
    TitleNode->Color = color;
    return true;
  }
  // ResizeBorder 同时影响行为命中宽度与滚动条避让距离；这是 Panel 视觉组装
  // 唯一需要消费的能力属性，其余属性由 BaseNode 自动转发给相应 Behavior。
  if (name == "ResizeBorder") {
    auto *resize = GetBehavior<ResizeBehavior>();
    if (!resize || !resize->SetProperty(name, value))
      return false;
    ScrollAreaNode->SetVerticalBarInsets(
        2.0f, resize->Properties.Border + 2.0f, 2.0f);
    return true;
  }
  // 只把滚动能力属性委托给组合子节点；Id、尺寸等通用属性仍必须作用于 Panel，
  // 否则声明键会错误写入内部 ScrollNode 并破坏协调器索引。
  if (auto *scroll = ScrollAreaNode->GetScrollBehavior();
      scroll && scroll->SetProperty(name, value))
    return true;
  return RectNode::SetProperty(name, value);
}
