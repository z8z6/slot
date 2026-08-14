#include "UI/Layout/PanelGroupNode.h"

#include "UI/Behavior/DockBehavior.h"
#include "UI/Behavior/DragBehavior.h"
#include "UI/Behavior/ResizeBehavior.h"
#include "UI/Style/Theme.h"
#include "Util/Color.h"

#include <algorithm>
#include <dwrite.h>
#include <limits>
#include <utility>
#include <windows.h>
#include <wrl/client.h>

using namespace z8::ui;

namespace {

IDWriteFactory *GetTextFactory() {
  static Microsoft::WRL::ComPtr<IDWriteFactory> factory = [] {
    Microsoft::WRL::ComPtr<IDWriteFactory> result;
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                        reinterpret_cast<IUnknown **>(result.GetAddressOf()));
    return result;
  }();
  return factory.Get();
}

float MeasureTabTextWidth(const std::string &text,
                          const std::wstring &fontFamily, float fontSize) {
  const int characterCount =
      MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
                          static_cast<int>(text.size()), nullptr, 0);
  std::wstring wideText(static_cast<size_t>((std::max)(characterCount, 0)),
                        L'\0');
  if (characterCount > 0)
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(),
                        static_cast<int>(text.size()), wideText.data(),
                        characterCount);

  float glyphWidth = static_cast<float>(wideText.size()) * fontSize * 0.56f;
  Microsoft::WRL::ComPtr<IDWriteTextFormat> format;
  Microsoft::WRL::ComPtr<IDWriteTextLayout> layout;
  // 页签宽度与最终文本通道共用 Theme 字体和 DirectWrite 度量，避免更换字体后
  // 绘制字形宽度与命中区域脱节。创建失败时保留确定性回退，使无图形设备的
  // 布局测试仍可运行。
  auto *factory = GetTextFactory();
  if (factory &&
      SUCCEEDED(factory->CreateTextFormat(
          fontFamily.c_str(), nullptr, DWRITE_FONT_WEIGHT_NORMAL,
          DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize,
          L"zh-cn", format.GetAddressOf())) &&
      SUCCEEDED(factory->CreateTextLayout(
          wideText.data(), static_cast<UINT32>(wideText.size()), format.Get(),
          4096.0f, fontSize * 2.0f, layout.GetAddressOf()))) {
    DWRITE_TEXT_METRICS metrics{};
    if (SUCCEEDED(layout->GetMetrics(&metrics)))
      glyphWidth = metrics.widthIncludingTrailingWhitespace;
  }
  // 图标、间距和左右留白属于页签固有宽度；这些比例与最终状态色同由
  // TabStyle 管理，标题变化不会重新引入控件局部 Magic Number。
  const auto &tab = Theme::Default().Tab;
  return std::clamp(glyphWidth + tab.FixedContentWidth, tab.MinWidth,
                    tab.MaxWidth);
}

WidgetVisualState ResolveInteractiveState(const BehaviorNode &node,
                                          bool selected = false) {
  if (node.Pressed)
    return WidgetVisualState::Pressed;
  if (node.Hovered)
    return WidgetVisualState::Hovered;
  return selected ? WidgetVisualState::Selected : WidgetVisualState::Normal;
}

} // namespace

PanelGroupCloseNode::PanelGroupCloseNode(PanelGroupNode &group)
    : Group(&group) {
  const auto &theme = Theme::Default();
  Style.Margin = 0.0f;
  Style.Padding = 0.0f;
  Style.Width = theme.Button.IconButtonSize;
  Style.Height = theme.Tab.Height;
  Style.MinWidth = 0.0f;
  Style.MinHeight = 0.0f;
  Style.FlexGrow = 0.0f;
  Style.FlexShrink = 0.0f;
  SetBorder(theme.Button.BorderColor, theme.Button.BorderWidth);
  SetCornerRadius(theme.Button.CornerRadius);

  auto icon = std::make_unique<ImageNode>();
  IconNode = icon.get();
  IconNode->Key = "__close_icon";
  IconNode->SetIcon(UIIcon::Close);
  IconNode->HitTestVisible = false;
  IconNode->Style.Width = theme.Icon.SmallSize;
  IconNode->Style.Height = theme.Icon.SmallSize;
  IconNode->Style.Margin =
      (theme.Button.IconButtonSize - theme.Icon.SmallSize) / 2.0f;
  BaseNode::AddChild(std::move(icon));
  OnVisualStateChanged();
}

void PanelGroupCloseNode::OnVisualStateChanged() {
  const auto &button = Theme::Default().Button;
  const auto state = ResolveInteractiveState(*this);
  SetColor(button.BackgroundColor.Resolve(state));
  if (IconNode)
    IconNode->SetColor(button.ForegroundColor.Resolve(state));
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
  Style.Height = theme.Tab.Height;
  Style.MinWidth = theme.Tab.MinWidth;
  Style.MaxWidth = theme.Tab.MaxWidth;
  Style.FlexGrow = 0.0f;
  Style.FlexShrink = 0.0f;
  SetBorder(theme.Tab.SeparatorColor, 0.0f);
  SetCornerRadius(theme.Tab.CornerRadius);

  auto icon = std::make_unique<ImageNode>();
  IconNode = icon.get();
  IconNode->Key = "__tab_icon";
  SetIcon(iconSource);
  IconNode->HitTestVisible = false;
  IconNode->Style.Width = theme.Tab.IconSize;
  IconNode->Style.Height = theme.Tab.IconSize;
  IconNode->Style.Margin = theme.Tab.IconMargin;
  BaseNode::AddChild(std::move(icon));

  auto label = std::make_unique<TextNode>(title);
  LabelNode = label.get();
  LabelNode->Key = "__tab_label";
  LabelNode->Alignment = TextAlignment::Center;
  LabelNode->Color =
      theme.Tab.ForegroundColor.Resolve(WidgetVisualState::Normal);
  LabelNode->Style.Margin = 0.0f;
  LabelNode->Style.FlexGrow = 1.0f;
  BaseNode::AddChild(std::move(label));

  auto selection = std::make_unique<RectNode>();
  SelectionNode = selection.get();
  SelectionNode->Key = "__selection";
  SelectionNode->HitTestVisible = false;
  SelectionNode->SetColor(theme.Tab.AccentColor);
  SelectionNode->SetBorder(theme.Tab.AccentColor, 0.0f);
  SelectionNode->Style.Position = PositionType::Absolute;
  SelectionNode->Style.Left = 0.0f;
  SelectionNode->Style.Right = 0.0f;
  SelectionNode->Style.Bottom = 0.0f;
  SelectionNode->Style.Height = theme.Tab.AccentHeight;
  SelectionNode->Style.MinWidth = 0.0f;
  SelectionNode->Style.MinHeight = 0.0f;
  BaseNode::AddChild(std::move(selection));

  SetTitle(title);

  // Tab 也使用通用 DragBehavior 的阈值与捕获生命周期；Layout 根据命中
  // 的 Tab 固定 Payload=Panel，不会在移动过程中切换为 Group。
  auto *drag = AddBehavior<DragBehavior>();
  drag->SetHandle(this);
  drag->SetPreviewOnly(true);
  OnVisualStateChanged();
}

void PanelGroupTabNode::OnVisualStateChanged() {
  const auto &tab = Theme::Default().Tab;
  const bool selected = Group && PanelIndex == Group->ActivePanel;
  const auto state = ResolveInteractiveState(*this, selected);
  SetColor(tab.BackgroundColor.Resolve(state));
  if (LabelNode)
    LabelNode->Color = tab.ForegroundColor.Resolve(state);
  if (IconNode)
    IconNode->SetColor(tab.IconColor.Resolve(state));
}

void PanelGroupTabNode::RefreshVisualState() { OnVisualStateChanged(); }

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

bool PanelGroupTabNode::SetIcon(UIIcon icon) {
  return IconNode && IconNode->SetIcon(icon);
}

void PanelGroupTabNode::SetTitle(const std::string &title) {
  if (!LabelNode)
    return;
  LabelNode->Text = title;
  const auto &textStyle = Theme::Default().Text;
  Style.Width =
      MeasureTabTextWidth(title, textStyle.FontFamily, textStyle.FontSize);
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
  HeaderNode->SetColor(style.TitleActiveColor);
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
  TabBarNode->SetColor(style.TitleActiveColor);
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
  DragHandleNode->SetColor(style.TitleActiveColor);
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

  auto separator = std::make_unique<RectNode>();
  ContentSeparatorNode = separator.get();
  ContentSeparatorNode->Key = "__content_separator";
  ContentSeparatorNode->HitTestVisible = false;
  ContentSeparatorNode->SetColor(style.ContentSeparatorColor);
  ContentSeparatorNode->SetBorder(style.ContentSeparatorColor, 0.0f);
  ContentSeparatorNode->Style.Margin = 0.0f;
  ContentSeparatorNode->Style.Padding = 0.0f;
  ContentSeparatorNode->Style.Height = style.ContentSeparatorWidth;
  ContentSeparatorNode->Style.MinHeight = 0.0f;
  ContentSeparatorNode->Style.FlexGrow = 0.0f;
  ContentSeparatorNode->Style.FlexShrink = 0.0f;
  BaseNode::AddChild(std::move(separator));

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

  // 空白标题区只承担视觉填充；Dock 手势必须从具体 Panel tab 开始，避免
  // 用户移动 Floating 原生窗口时意外触发整组 Dock 预览。Floating host 会把
  // 该区域映射为 HTCAPTION，Docked Group 点击这里则保持静默。
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
  // 停靠尺寸约束属于外层 Group；页节点只负责填满扣除公共标题栏后的 Pages。
  // 若保留 Panel 的 MinHeight，较矮的顶部工具栏会在首帧把内容挤出可见区。
  result->Style.MinWidth = 0.0f;
  result->Style.MinHeight = 0.0f;
  result->Style.MaxWidth = (std::numeric_limits<float>::max)();
  result->Style.MaxHeight = (std::numeric_limits<float>::max)();
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
  // 脱离 Group 后重新承担外层窗口职责，恢复 Group 当前的尺寸约束；这些
  // 约束可能已经由 XAML 热更新或用户属性修改，不能回退到主题默认值。
  panelOwner->Style.MinWidth = Style.MinWidth;
  panelOwner->Style.MinHeight = Style.MinHeight;
  panelOwner->Style.MaxWidth = Style.MaxWidth;
  panelOwner->Style.MaxHeight = Style.MaxHeight;

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
  for (size_t index = 0; index < Panels.size(); ++index) {
    const bool active = index == ActivePanel;
    Panels[index]->Visible = active;
    Tabs[index]->SelectionNode->Visible = active;
    Tabs[index]->RefreshVisualState();
  }
}
