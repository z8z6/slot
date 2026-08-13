//
// Created by zhou_zhengming on 2026/7/31.
//

#include "UI/Layout/PanelNode.h"

#include "UI/Layout/PanelGroupNode.h"
#include "UI/Style/Theme.h"
#include "Util/Color.h"

#include <algorithm>
#include <cstdlib>

using namespace z8::ui;

PanelNode::PanelNode()
    : TitleIconNode(nullptr), TitleBarNode(nullptr), TitleNode(nullptr),
      ScrollAreaNode(nullptr) {
  const auto &style = Theme::Default().Panel;
  // Panel 自身纵向排列；标题栏固定高度，内容宿主占据剩余空间。
  Style.Direction = FlexDirection::Column;
  SetColor(style.Color);
  SetBorder(style.BorderColor, style.BorderWidth);
  Style.Margin = style.Margin;
  Style.Padding = style.Padding;
  Style.MinWidth = style.MinWidth;
  Style.MinHeight = style.MinHeight;
  TitleHeight = style.TitleHeight;

  auto titleBar = std::make_unique<RectNode>();
  TitleBarNode = titleBar.get();
  TitleBarNode->Key = "__title_bar";
  TitleBarNode->SetColor(style.TitleColor);
  // 标题栏使用独立视觉节点，否则 TitleColor 只是未被消费的主题
  // 数据，标题与内容会共用 Panel 背景而无法形成层级。
  TitleBarNode->SetBorder(style.BorderColor, 0.0f);
  TitleBarNode->Style.Margin = 0.0f;
  TitleBarNode->Style.Padding = 0.0f;
  TitleBarNode->Style.Direction = FlexDirection::Row;
  TitleBarNode->Style.Height = TitleHeight;
  TitleBarNode->Style.FlexGrow = 0.0f;
  TitleBarNode->Style.FlexShrink = 0.0f;

  auto icon = std::make_unique<ImageNode>();
  TitleIconNode = icon.get();
  TitleIconNode->Key = "__title_icon";
  TitleIconNode->SetProperty("Source", IconSource);
  TitleIconNode->SetColor(style.TitleTextColor);
  TitleIconNode->HitTestVisible = false;
  TitleIconNode->Style.Margin = 7.0f;
  TitleBarNode->BaseNode::AddChild(std::move(icon));

  auto title = std::make_unique<TextNode>();
  TitleNode = title.get();
  TitleNode->Key = "__title";
  TitleNode->Color = style.TitleTextColor;
  TitleNode->Alignment = TextAlignment::Center;
  // 标题文字独占标题栏布局框；背景仍由 Panel 绘制，避免为标题额外创建矩形。
  TitleNode->Style.Margin = 0.0f;
  TitleNode->Style.Height = TitleHeight;
  TitleNode->Style.FlexGrow = 1.0f;
  TitleNode->Style.FlexShrink = 1.0f;
  TitleBarNode->BaseNode::AddChild(std::move(title));
  BaseNode::AddChild(std::move(titleBar));

  auto scroll = std::make_unique<z8::ui::ScrollNode>();
  ScrollAreaNode = scroll.get();
  ScrollAreaNode->Key = "__scroll";
  ScrollAreaNode->Style.FlexGrow = 1.0f;
  ScrollAreaNode->Style.FlexShrink = 1.0f;
  ScrollAreaNode->SetVerticalBarInsets(2.0f, 0.0f, 2.0f);
  BaseNode::AddChild(std::move(scroll));

  // Panel 只组装视觉和能力。行为优先级由组件自身声明，因此边缘 Resize 会在
  // 标题 Drag 之前获得同一次按下，而 Panel 不需要知道仲裁细节。
  auto *drag = AddBehavior<DragBehavior>();
  drag->SetHandle(TitleBarNode);
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
  if (Group && (name == "Dock" || name == "DockEnabled" ||
                name == "DockThreshold" || name == "DockExtent" ||
                name == "Width" || name == "Height" ||
                name == "MinWidth" || name == "MinHeight" ||
                name == "MaxWidth" || name == "MaxHeight" ||
                name == "Margin"))
    return Group->RectNode::SetProperty(name, value);
  if (name == "Icon" || name == "TitleIcon") {
    if (!TitleIconNode->SetProperty("Source", value))
      return false;
    IconSource = value;
    if (Group) {
      const auto panel = std::find(Group->Panels.begin(), Group->Panels.end(),
                                   this);
      if (panel != Group->Panels.end())
        Group->Tabs[static_cast<size_t>(panel - Group->Panels.begin())]
            ->SetIcon(value);
    }
    return true;
  }
  if (name == "Title") {
    // 即时声明每帧会重复提交相同标题；相同值直接复用现有 DirectWrite 度量。
    if (TitleNode->Text == value)
      return true;
    TitleNode->Text = value;
    if (Group) {
      const auto panel = std::find(Group->Panels.begin(), Group->Panels.end(),
                                   this);
      if (panel != Group->Panels.end())
        Group->Tabs[static_cast<size_t>(panel - Group->Panels.begin())]
            ->SetTitle(value);
    }
    return true;
  }
  if (name == "TitleHeight") {
    TitleHeight = std::strtof(value.c_str(), nullptr);
    TitleBarNode->Style.Height = TitleHeight;
    TitleNode->Style.Height = TitleHeight;
    return true;
  }
  if (name == "TitleColor") {
    DirectX::XMFLOAT4 color;
    if (!ParseUIColor(value, color))
      return false;
    TitleBarNode->SetColor(color);
    return true;
  }
  if (name == "TitleTextColor") {
    DirectX::XMFLOAT4 color;
    if (!ParseUIColor(value, color))
      return false;
    TitleNode->Color = color;
    return true;
  }
  // ResizeBorder 只改变边缘命中范围；滚动条保持贴齐内容区右侧，不能随命中
  // 宽度向左漂移。其余属性由 BaseNode 自动转发给相应 Behavior。
  if (name == "ResizeBorder") {
    auto *resize = GetBehavior<ResizeBehavior>();
    if (!resize || !resize->SetProperty(name, value))
      return false;
    ScrollAreaNode->SetVerticalBarInsets(2.0f, 0.0f, 2.0f);
    return true;
  }
  // 只把滚动能力属性委托给组合子节点；Id、尺寸等通用属性仍必须作用于 Panel，
  // 否则声明键会错误写入内部 ScrollNode 并破坏协调器索引。
  if (auto *scroll = ScrollAreaNode->GetScrollBehavior();
      scroll && scroll->SetProperty(name, value))
    return true;
  return RectNode::SetProperty(name, value);
}
