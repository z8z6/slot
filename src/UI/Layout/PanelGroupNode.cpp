#include "UI/Layout/PanelGroupNode.h"

#include "UI/Behavior/DockBehavior.h"
#include "UI/Behavior/DragBehavior.h"
#include "UI/Behavior/ResizeBehavior.h"
#include "UI/Style/Theme.h"
#include "Util/Color.h"

#include <utility>

using namespace z8::ui;

PanelGroupTabNode::PanelGroupTabNode(PanelGroupNode &group, size_t panelIndex,
                                     const std::string &title)
    : Group(&group), PanelIndex(panelIndex) {
  const auto &theme = Theme::Default();
  Style.Margin = 0.0f;
  Style.Padding = 0.0f;
  Style.MinWidth = 72.0f;
  Style.FlexGrow = 1.0f;
  Style.FlexShrink = 1.0f;
  SetBorder(theme.Panel.BorderColor, 1.0f);

  auto label = std::make_unique<TextNode>(title);
  LabelNode = label.get();
  LabelNode->Key = "__tab_label";
  LabelNode->Alignment = TextAlignment::Center;
  LabelNode->Color = theme.Panel.TitleTextColor;
  LabelNode->Style.Margin = 0.0f;
  LabelNode->Style.FlexGrow = 1.0f;
  BaseNode::AddChild(std::move(label));
}

z8::EventReply PanelGroupTabNode::OnMouseDown(MouseMovArgs args) {
  if (args.Button != MouseButton::Left || !Contains(args) || !Group)
    return z8::EventReply::Ignored;
  Group->ActivatePanel(PanelIndex);
  // 单击先切换页签，再继续冒泡给 Group 的 DragBehavior，使标题页
  // 同时支持切换和拖动整组。
  return z8::EventReply::Ignored;
}

PanelGroupNode::PanelGroupNode() {
  const auto &style = Theme::Default().Panel;
  Style.Direction = FlexDirection::Column;
  Style.Margin = style.Margin;
  Style.Padding = 0.0f;
  Style.MinWidth = style.MinWidth;
  Style.MinHeight = style.MinHeight;
  SetColor(style.Color);
  SetBorder(style.BorderColor, style.BorderWidth);

  auto tabBar = std::make_unique<RectNode>();
  TabBarNode = tabBar.get();
  TabBarNode->Key = "__tabs";
  TabBarNode->SetColor(style.TitleColor);
  TabBarNode->SetBorder(style.BorderColor, 0.0f);
  TabBarNode->Style.Direction = FlexDirection::Row;
  TabBarNode->Style.Margin = 0.0f;
  TabBarNode->Style.Padding = 0.0f;
  TabBarNode->Style.Height = style.TitleHeight;
  TabBarNode->Style.FlexGrow = 0.0f;
  TabBarNode->Style.FlexShrink = 0.0f;
  BaseNode::AddChild(std::move(tabBar));

  auto pages = std::make_unique<BaseNode>();
  PagesNode = pages.get();
  PagesNode->Key = "__pages";
  PagesNode->Style.Margin = 0.0f;
  PagesNode->Style.Padding = 0.0f;
  PagesNode->Style.FlexGrow = 1.0f;
  PagesNode->Style.FlexShrink = 1.0f;
  BaseNode::AddChild(std::move(pages));

  // 组作为一个整体参与 Dock；页签按钮会先消费点击，标题栏
  // 空白区则仍可拖动整个 Group。
  auto *drag = AddBehavior<DragBehavior>();
  drag->SetHandle(TabBarNode);
  auto *resize = AddBehavior<ResizeBehavior>();
  ResizeProperty resizeProperty;
  resizeProperty.MinWidth = style.MinWidth;
  resizeProperty.MinHeight = style.MinHeight;
  resizeProperty.Border = style.ResizeBorder;
  resize->SetProperties(resizeProperty);
  AddBehavior<DockBehavior>();
}

bool PanelGroupNode::ActivatePanel(size_t panelIndex) {
  if (panelIndex >= Panels.size())
    return false;
  ActivePanel = panelIndex;
  UpdateSelectionVisuals();
  return true;
}

BaseNode *PanelGroupNode::AddChild(std::unique_ptr<BaseNode> child) {
  if (!child)
    return nullptr;
  auto *panel = dynamic_cast<PanelNode *>(child.get());
  if (!panel)
    return nullptr;
  child.release();
  return AddPanel(std::unique_ptr<PanelNode>(panel));
}

PanelNode *PanelGroupNode::AddPanel(std::unique_ptr<PanelNode> panel) {
  if (!panel || !PagesNode || !TabBarNode)
    return nullptr;
  auto *result = panel.get();
  const size_t index = Panels.size();

  // 组内 Panel 不再绘制自身单标题栏，并以绝对四边约束共享同一
  // Pages 矩形。不活动页仅切换可见性，不销毁其滚动等运行状态。
  result->TitleBarNode->Visible = false;
  result->TitleBarNode->Style.Height = 0.0f;
  result->LayoutManaged = true;
  result->Group = this;
  result->Style.Margin = 0.0f;
  result->Style.Padding = 0.0f;
  result->Style.Position = PositionType::Absolute;
  result->Style.Left = 0.0f;
  result->Style.Top = 0.0f;
  result->Style.Right = 0.0f;
  result->Style.Bottom = 0.0f;
  result->Style.Width.reset();
  result->Style.Height.reset();
  result->Visible = index == ActivePanel;

  auto tab = std::make_unique<PanelGroupTabNode>(
      *this, index,
      result->TitleNode->Text.empty() ? result->Key : result->TitleNode->Text);
  auto *tabObserver = tab.get();
  TabBarNode->BaseNode::AddChild(std::move(tab));
  PagesNode->BaseNode::AddChild(std::move(panel));
  Panels.push_back(result);
  Tabs.push_back(tabObserver);
  UpdateSelectionVisuals();
  return result;
}

bool PanelGroupNode::HasInteractiveGeometry() const {
  const auto *drag = GetBehavior<DragBehavior>();
  const auto *resize = GetBehavior<ResizeBehavior>();
  return (drag && drag->HasInteractiveGeometry()) ||
         (resize && resize->HasInteractiveGeometry());
}

void PanelGroupNode::MergeFrom(PanelGroupNode &source) {
  if (&source == this || !source.PagesNode)
    return;
  std::vector<std::unique_ptr<PanelNode>> movedPanels;
  movedPanels.reserve(source.PagesNode->Children.size());
  for (auto &child : source.PagesNode->Children) {
    auto *panel = dynamic_cast<PanelNode *>(child.release());
    if (panel)
      movedPanels.emplace_back(panel);
  }
  source.PagesNode->Children.clear();
  source.TabBarNode->Children.clear();
  source.Panels.clear();
  source.Tabs.clear();
  const size_t firstMoved = Panels.size();
  for (auto &panel : movedPanels)
    AddPanel(std::move(panel));
  if (firstMoved < Panels.size())
    ActivatePanel(firstMoved);
}

bool PanelGroupNode::SetProperty(const std::string &name,
                                 const std::string &value) {
  if (RectNode::SetProperty(name, value))
    return true;
  return Panels.size() == 1 && Panels.front()->SetProperty(name, value);
}

void PanelGroupNode::UpdateSelectionVisuals() {
  const auto &theme = Theme::Default();
  for (size_t index = 0; index < Panels.size(); ++index) {
    const bool active = index == ActivePanel;
    Panels[index]->Visible = active;
    Tabs[index]->SetColor(active ? theme.Panel.TitleColor
                                 : Color::ControlPressed);
  }
}
