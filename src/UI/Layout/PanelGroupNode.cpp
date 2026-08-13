#include "UI/Layout/PanelGroupNode.h"

#include "UI/Behavior/DockBehavior.h"
#include "UI/Behavior/DragBehavior.h"
#include "UI/Behavior/ResizeBehavior.h"
#include "UI/Style/Theme.h"
#include "Util/Color.h"

#include <algorithm>
#include <dwrite.h>
#include <wrl/client.h>
#include <utility>
#include <windows.h>

using namespace z8::ui;

namespace {

IDWriteFactory *GetTextFactory() {
  static Microsoft::WRL::ComPtr<IDWriteFactory> factory = [] {
    Microsoft::WRL::ComPtr<IDWriteFactory> result;
    DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown **>(result.GetAddressOf()));
    return result;
  }();
  return factory.Get();
}

float MeasureTabTextWidth(const std::string &text, float fontSize) {
  const int characterCount = MultiByteToWideChar(
      CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()),
      nullptr, 0);
  std::wstring wideText(static_cast<size_t>((std::max)(characterCount, 0)),
                        L'\0');
  if (characterCount > 0)
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
                        static_cast<int>(text.size()), wideText.data(),
                        characterCount);

  float glyphWidth = static_cast<float>(wideText.size()) * fontSize * 0.56f;
  Microsoft::WRL::ComPtr<IDWriteTextFormat> format;
  Microsoft::WRL::ComPtr<IDWriteTextLayout> layout;
  // 页签宽度与最终文本通道共用 Segoe UI/DirectWrite 度量，避免宽窄字形标题
  // 依赖经验系数。创建失败时保留确定性回退，使无图形设备的布局测试仍可运行。
  auto *factory = GetTextFactory();
  if (factory && SUCCEEDED(factory->CreateTextFormat(
          L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
          DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize,
          L"zh-cn", format.GetAddressOf())) &&
      SUCCEEDED(factory->CreateTextLayout(
          wideText.data(), static_cast<UINT32>(wideText.size()), format.Get(),
          4096.0f, fontSize * 2.0f, layout.GetAddressOf()))) {
    DWRITE_TEXT_METRICS metrics{};
    if (SUCCEEDED(layout->GetMetrics(&metrics)))
      glyphWidth = metrics.widthIncludingTrailingWhitespace;
  }
  // 16px 图标、8px 间距和左右留白都属于页签固有宽度。
  constexpr float iconAndPadding = 48.0f;
  return std::clamp(glyphWidth + iconAndPadding, 72.0f, 240.0f);
}

} // namespace

PanelGroupCloseNode::PanelGroupCloseNode(PanelGroupNode &group) : Group(&group) {
  const auto &theme = Theme::Default();
  Style.Margin = 0.0f;
  Style.Padding = 0.0f;
  Style.Width = theme.Panel.TitleHeight;
  Style.Height = theme.Panel.TitleHeight;
  Style.MinWidth = 0.0f;
  Style.MinHeight = 0.0f;
  Style.FlexGrow = 0.0f;
  Style.FlexShrink = 0.0f;
  SetColor(Color::ControlPressed);
  SetBorder(theme.Panel.BorderColor, 1.0f);
  SetCornerRadius(theme.Rect.CornerRadius);

  auto icon = std::make_unique<ImageNode>();
  IconNode = icon.get();
  IconNode->Key = "__close_icon";
  IconNode->SetProperty("Source", "asset://texture/icons/lucide/x.svg");
  IconNode->SetColor(theme.Panel.TitleTextColor);
  IconNode->HitTestVisible = false;
  IconNode->Style.Margin = 8.0f;
  BaseNode::AddChild(std::move(icon));
}

z8::EventReply PanelGroupCloseNode::OnMouseDown(MouseMovArgs args) {
  if (args.Button != MouseButton::Left || !Contains(args) || !Group)
    return z8::EventReply::Ignored;
  // 只记录意图，Layout 会先撤销 Dock/捕获状态再销毁 Group，避免在
  // 节点自己的事件栈中释放 this。
  Group->CloseRequested = true;
  return z8::EventReply::Handled;
}

PanelGroupTabNode::PanelGroupTabNode(PanelGroupNode &group, size_t panelIndex,
                                     const std::string &title,
                                     const std::string &iconSource)
    : Group(&group), PanelIndex(panelIndex) {
  const auto &theme = Theme::Default();
  Style.Margin = 0.0f;
  Style.Padding = 0.0f;
  Style.Direction = FlexDirection::Row;
  Style.MinWidth = 72.0f;
  Style.MaxWidth = 240.0f;
  Style.FlexGrow = 0.0f;
  Style.FlexShrink = 0.0f;
  SetColor(Color::ControlPressed);
  SetBorder(theme.Panel.BorderColor, 1.0f);
  SetCornerRadius(theme.Rect.CornerRadius);

  auto icon = std::make_unique<ImageNode>();
  IconNode = icon.get();
  IconNode->Key = "__tab_icon";
  SetIcon(iconSource);
  IconNode->SetColor(theme.Panel.TitleTextColor);
  IconNode->HitTestVisible = false;
  IconNode->Style.Width = 16.0f;
  IconNode->Style.Height = 16.0f;
  IconNode->Style.Margin = 6.0f;
  BaseNode::AddChild(std::move(icon));

  auto label = std::make_unique<TextNode>(title);
  LabelNode = label.get();
  LabelNode->Key = "__tab_label";
  LabelNode->Alignment = TextAlignment::Center;
  LabelNode->Color = theme.Panel.TitleTextColor;
  LabelNode->Style.Margin = 0.0f;
  LabelNode->Style.FlexGrow = 1.0f;
  BaseNode::AddChild(std::move(label));

  auto selection = std::make_unique<RectNode>();
  SelectionNode = selection.get();
  SelectionNode->Key = "__selection";
  SelectionNode->HitTestVisible = false;
  SelectionNode->SetColor(Color::Accent);
  SelectionNode->SetBorder(Color::Accent, 0.0f);
  SelectionNode->Style.Position = PositionType::Absolute;
  SelectionNode->Style.Left = 0.0f;
  SelectionNode->Style.Right = 0.0f;
  SelectionNode->Style.Bottom = 0.0f;
  SelectionNode->Style.Height = 2.0f;
  SelectionNode->Style.MinWidth = 0.0f;
  SelectionNode->Style.MinHeight = 0.0f;
  BaseNode::AddChild(std::move(selection));

  SetTitle(title);

  // Tab 也使用通用 DragBehavior 的阈值与捕获生命周期；Layout 根据命中
  // 的 Tab 固定 Payload=Panel，不会在移动过程中切换为 Group。
  auto *drag = AddBehavior<DragBehavior>();
  drag->SetHandle(this);
  drag->SetPreviewOnly(true);
}

z8::EventReply PanelGroupTabNode::OnMouseDown(MouseMovArgs args) {
  if (args.Button != MouseButton::Left || !Contains(args) || !Group)
    return z8::EventReply::Ignored;
  Group->ActivatePanel(PanelIndex);
  // Tab 必须终止冒泡，不能借由 Group DragBehavior 捕获同一次按下；
  // 当前方案明确不支持拖出或重排单个 Tab。
  return z8::EventReply::Handled;
}

bool PanelGroupTabNode::SetIcon(const std::string &iconSource) {
  return IconNode && IconNode->SetProperty("Source", iconSource);
}

void PanelGroupTabNode::SetTitle(const std::string &title) {
  if (!LabelNode)
    return;
  LabelNode->Text = title;
  Style.Width = MeasureTabTextWidth(title, Theme::Default().Text.FontSize);
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
  SetCornerRadius(style.CornerRadius);

  auto header = std::make_unique<RectNode>();
  HeaderNode = header.get();
  HeaderNode->Key = "__header";
  HeaderNode->SetColor(style.TitleColor);
  HeaderNode->SetBorder(style.BorderColor, 0.0f);
  HeaderNode->Style.Direction = FlexDirection::Row;
  HeaderNode->Style.Margin = 0.0f;
  HeaderNode->Style.Padding = 0.0f;
  HeaderNode->Style.Height = style.TitleHeight;
  HeaderNode->Style.FlexGrow = 0.0f;
  HeaderNode->Style.FlexShrink = 0.0f;

  auto tabBar = std::make_unique<RectNode>();
  TabBarNode = tabBar.get();
  TabBarNode->Key = "__tabs";
  TabBarNode->SetColor(style.TitleColor);
  TabBarNode->SetBorder(style.BorderColor, 0.0f);
  TabBarNode->Style.Direction = FlexDirection::Row;
  TabBarNode->Style.Margin = 0.0f;
  TabBarNode->Style.Padding = 0.0f;
  TabBarNode->Style.Height = style.TitleHeight;
  TabBarNode->Style.MinWidth = 0.0f;
  TabBarNode->Style.FlexGrow = 0.0f;
  TabBarNode->Style.FlexShrink = 1.0f;
  HeaderNode->BaseNode::AddChild(std::move(tabBar));

  auto dragHandle = std::make_unique<RectNode>();
  DragHandleNode = dragHandle.get();
  DragHandleNode->Key = "__drag_handle";
  DragHandleNode->SetColor(style.TitleColor);
  DragHandleNode->SetBorder(style.BorderColor, 0.0f);
  DragHandleNode->Style.Margin = 0.0f;
  DragHandleNode->Style.Padding = 0.0f;
  DragHandleNode->Style.MinWidth = 24.0f;
  DragHandleNode->Style.Height = style.TitleHeight;
  DragHandleNode->Style.FlexGrow = 1.0f;
  DragHandleNode->Style.FlexShrink = 1.0f;
  HeaderNode->BaseNode::AddChild(std::move(dragHandle));

  auto close = std::make_unique<PanelGroupCloseNode>(*this);
  CloseButtonNode = close.get();
  CloseButtonNode->Key = "__close";
  HeaderNode->BaseNode::AddChild(std::move(close));
  BaseNode::AddChild(std::move(header));

  auto pages = std::make_unique<BaseNode>();
  PagesNode = pages.get();
  PagesNode->Key = "__pages";
  PagesNode->Style.Margin = 0.0f;
  PagesNode->Style.Padding = 0.0f;
  PagesNode->Style.FlexGrow = 1.0f;
  PagesNode->Style.FlexShrink = 1.0f;
  // 页面使用绝对四边约束，显式裁剪可确保内容永远不会越过 Group
  // Content Rect 覆盖标题栏或相邻 Dock 区域。
  PagesNode->ClipChildren = true;
  BaseNode::AddChild(std::move(pages));

  // 只有 Tab 与关闭按钮之间的空白标题区可启动 Group 拖动，因此输入
  // 优先级不依赖 Behavior 注册顺序，Panel 内容也不会误触发拖动。
  auto *drag = AddBehavior<DragBehavior>();
  drag->SetHandle(DragHandleNode);
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
      result->TitleNode->Text.empty() ? result->Key : result->TitleNode->Text,
      result->IconSource);
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

bool PanelGroupNode::SwapPanels(size_t firstIndex, size_t secondIndex) {
  if (firstIndex >= Panels.size() || secondIndex >= Panels.size())
    return false;
  if (firstIndex == secondIndex)
    return true;
  auto *activePanel = Panels[ActivePanel];
  // Panel 所有权、Tab 所有权和快速观察数组必须作为同一事务交换，否则下一次
  // RemovePanel 会按错误索引释放对象，留下悬空的 PanelIndex 或 Group 指针。
  std::swap(PagesNode->Children[firstIndex], PagesNode->Children[secondIndex]);
  std::swap(TabBarNode->Children[firstIndex],
            TabBarNode->Children[secondIndex]);
  std::swap(Panels[firstIndex], Panels[secondIndex]);
  std::swap(Tabs[firstIndex], Tabs[secondIndex]);
  for (size_t index = 0; index < Tabs.size(); ++index) {
    Tabs[index]->PanelIndex = index;
    if (Panels[index] == activePanel)
      ActivePanel = index;
  }
  UpdateSelectionVisuals();
  return true;
}

std::unique_ptr<PanelNode> PanelGroupNode::RemovePanel(size_t panelIndex) {
  if (panelIndex >= Panels.size() || panelIndex >= PagesNode->Children.size() ||
      panelIndex >= TabBarNode->Children.size())
    return nullptr;

  auto panelOwner = std::unique_ptr<PanelNode>(
      static_cast<PanelNode *>(PagesNode->Children[panelIndex].release()));
  PagesNode->Children.erase(PagesNode->Children.begin() +
                            static_cast<std::ptrdiff_t>(panelIndex));
  TabBarNode->Children.erase(TabBarNode->Children.begin() +
                             static_cast<std::ptrdiff_t>(panelIndex));
  Panels.erase(Panels.begin() + static_cast<std::ptrdiff_t>(panelIndex));
  Tabs.erase(Tabs.begin() + static_cast<std::ptrdiff_t>(panelIndex));

  panelOwner->Group = nullptr;
  panelOwner->Parent = nullptr;
  panelOwner->LayoutManaged = false;
  panelOwner->TitleBarNode->Visible = true;
  panelOwner->TitleBarNode->Style.Height = Theme::Default().Panel.TitleHeight;

  // 删除活动页时优先选择其右侧页；删除末页则自然回退到左侧相邻页。
  // 删除活动页之前的页面时同步左移索引，保持原活动 Panel 不变。
  if (Panels.empty()) {
    ActivePanel = 0;
    CloseRequested = true;
  } else if (panelIndex < ActivePanel) {
    --ActivePanel;
  } else if (ActivePanel >= Panels.size()) {
    ActivePanel = Panels.size() - 1;
  }
  for (size_t index = 0; index < Tabs.size(); ++index)
    Tabs[index]->PanelIndex = index;
  UpdateSelectionVisuals();
  return panelOwner;
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
    Tabs[index]->SetColor(active ? Color::ControlHover : Color::ControlPressed);
    Tabs[index]->LabelNode->Color = active ? theme.Panel.TitleTextColor
                                          : theme.Text.MutedColor;
    Tabs[index]->SelectionNode->Visible = active;
  }
}
