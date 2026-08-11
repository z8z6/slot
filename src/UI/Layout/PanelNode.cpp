//
// Created by zhou_zhengming on 2026/7/31.
//

#include "UI/Layout/PanelNode.h"

#include "Object/UIObject/RectUIObject.h"
#include "UI/Style/UITheme.h"
#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"

#include <algorithm>
#include <cstdlib>

using namespace z8::ui;

namespace {

bool ParseBoolean(const std::string& value, bool& result) {
  if (value == "true" || value == "True" || value == "1") {
    result = true;
    return true;
  }
  if (value == "false" || value == "False" || value == "0") {
    result = false;
    return true;
  }
  return false;
}

bool ParseScrollBarVisibility(const std::string& value,
                              ScrollBarVisibility& result) {
  if (value == "Hidden") result = ScrollBarVisibility::Hidden;
  else if (value == "Auto") result = ScrollBarVisibility::Auto;
  else if (value == "Visible") result = ScrollBarVisibility::Visible;
  else return false;
  return true;
}

} // namespace

PanelNode::PanelNode()
    : TitleNode(nullptr), ScrollViewportNode(nullptr), ContentNode(nullptr),
      VerticalScrollBarNode(nullptr), VerticalScrollThumbNode(nullptr) {
  const auto& style = UITheme::Modern().Panel;
  // Panel 自身纵向排列；标题栏固定高度，内容宿主占据剩余空间。
  YGNodeStyleSetFlexDirection(GetYogaNode(), YGFlexDirectionColumn);
  SetColor(style.Color);
  YGNodeStyleSetMargin(GetYogaNode(), YGEdgeAll, style.Margin);
  YGNodeStyleSetPadding(GetYogaNode(), YGEdgeAll, style.Padding);
  YGNodeStyleSetMinWidth(GetYogaNode(), style.MinimumWidth);
  YGNodeStyleSetMinHeight(GetYogaNode(), style.MinimumHeight);
  ResizeProperty resize = GetResizeProperties();
  resize.MinWidth = style.MinimumWidth;
  resize.MinHeight = style.MinimumHeight;
  resize.Border = style.ResizeBorder;
  IResizable::SetResizeProperties(resize);
  TitleHeight = style.TitleHeight;

  auto title = std::make_unique<RectNode>();
  TitleNode = title.get();
  TitleNode->Key = "__title";
  TitleNode->SetColor(style.TitleColor);
  // 内部标题不是普通列表项，清除 Rect 的外边距以贴合 Panel 边缘。
  YGNodeStyleSetMargin(TitleNode->GetYogaNode(), YGEdgeAll, 0.0f);
  YGNodeStyleSetHeight(TitleNode->GetYogaNode(), TitleHeight);
  YGNodeStyleSetFlexGrow(TitleNode->GetYogaNode(), 0.0f);
  YGNodeStyleSetFlexShrink(TitleNode->GetYogaNode(), 0.0f);
  BaseNode::AddChild(std::move(title));

  auto viewport = std::make_unique<BaseNode>();
  ScrollViewportNode = viewport.get();
  ScrollViewportNode->Key = "__scroll_viewport";
  YGNodeStyleSetFlexGrow(ScrollViewportNode->GetYogaNode(), 1.0f);
  YGNodeStyleSetFlexShrink(ScrollViewportNode->GetYogaNode(), 1.0f);
  ScrollViewportNode->SetClipsChildren(true);

  auto content = std::make_unique<BaseNode>();
  ContentNode = content.get();
  ContentNode->Key = "__content";
  // Content 按内容自然尺寸增长，Viewport 单独负责约束与裁剪。
  YGNodeStyleSetFlexGrow(ContentNode->GetYogaNode(), 0.0f);
  YGNodeStyleSetFlexShrink(ContentNode->GetYogaNode(), 0.0f);
  YGNodeStyleSetWidthPercent(ContentNode->GetYogaNode(), 100.0f);
  YGNodeStyleSetPadding(ContentNode->GetYogaNode(), YGEdgeAll,
                        style.ContentPadding);
  ScrollViewportNode->AddChild(std::move(content));
  BaseNode::AddChild(std::move(viewport));

  auto scrollBar = std::make_unique<ScrollBarNode>(
      UIScrollBarOrientation::Vertical);
  VerticalScrollBarNode = scrollBar.get();
  VerticalScrollBarNode->Key = "__vertical_scrollbar";
  YGNodeStyleSetPositionType(VerticalScrollBarNode->GetYogaNode(),
                             YGPositionTypeAbsolute);
  YGNodeStyleSetPosition(VerticalScrollBarNode->GetYogaNode(), YGEdgeTop,
                         TitleHeight + 2.0f);
  YGNodeStyleSetPosition(VerticalScrollBarNode->GetYogaNode(), YGEdgeRight,
                         GetResizeProperties().Border + 2.0f);
  YGNodeStyleSetPosition(VerticalScrollBarNode->GetYogaNode(), YGEdgeBottom,
                         2.0f);
  YGNodeStyleSetWidth(VerticalScrollBarNode->GetYogaNode(),
                      style.ScrollBarThickness);
  VerticalScrollThumbNode = VerticalScrollBarNode->ThumbNode;
  VerticalScrollBarNode->ValueChanged = [this](float value) {
    SetScrollOffsetY(value);
  };
  ApplyScrollProperties();
  BaseNode::AddChild(std::move(scrollBar));
}

BaseNode* PanelNode::ContentHost() { return ContentNode; }

void PanelNode::SetResizeProperties(const ResizeProperty& properties) {
  IResizable::SetResizeProperties(properties);
  YGNodeStyleSetMinWidth(GetYogaNode(), properties.MinWidth);
  YGNodeStyleSetMinHeight(GetYogaNode(), properties.MinHeight);
}

void PanelNode::SetScrollProperties(const ScrollProperty& properties) {
  IScrollable::SetScrollProperties(properties);
  ApplyScrollProperties();
}

bool PanelNode::SetProperty(const std::string& name, const std::string& value) {
  if (name == "Title") {
    // 当前渲染器尚无文字栅格化，先保留标题语义；标题栏几何已经可见。
    Title = value;
    return true;
  }
  if (name == "TitleHeight") {
    TitleHeight = std::strtof(value.c_str(), nullptr);
    YGNodeStyleSetHeight(TitleNode->GetYogaNode(), TitleHeight);
    YGNodeStyleSetPosition(VerticalScrollBarNode->GetYogaNode(), YGEdgeTop,
                           TitleHeight + 2.0f);
    return true;
  }
  if (name == "TitleColor") {
    DirectX::XMFLOAT4 color;
    if (!ParseUIColor(value, color)) return false;
    return TitleNode->SetColor(color);
  }
  if (name == "Draggable" || name == "DragEnabled") {
    DragProperty properties = GetDragProperties();
    if (!ParseBoolean(value, properties.Enabled)) return false;
    SetDragProperties(properties);
    return true;
  }
  if (name == "DragRegion") {
    DragProperty properties = GetDragProperties();
    if (value == "TitleBar") properties.Region = DragRegion::TitleBar;
    else if (value == "Anywhere")
      properties.Region = DragRegion::Anywhere;
    else return false;
    SetDragProperties(properties);
    return true;
  }
  if (name == "Resizable" || name == "ResizeEnabled") {
    ResizeProperty properties = GetResizeProperties();
    if (!ParseBoolean(value, properties.Enabled)) return false;
    SetResizeProperties(properties);
    return true;
  }
  if (name == "ResizeBorder") {
    ResizeProperty properties = GetResizeProperties();
    properties.Border = std::max(0.0f, std::strtof(value.c_str(), nullptr));
    SetResizeProperties(properties);
    YGNodeStyleSetPosition(VerticalScrollBarNode->GetYogaNode(), YGEdgeRight,
                           properties.Border + 2.0f);
    return true;
  }
  if (name == "MinWidth") {
    ResizeProperty properties = GetResizeProperties();
    properties.MinWidth = std::max(1.0f, std::strtof(value.c_str(), nullptr));
    SetResizeProperties(properties);
    return true;
  }
  if (name == "MinHeight") {
    ResizeProperty properties = GetResizeProperties();
    properties.MinHeight = std::max(1.0f, std::strtof(value.c_str(), nullptr));
    SetResizeProperties(properties);
    return true;
  }
  if (name == "Scrollable" || name == "ScrollEnabled") {
    ScrollProperty properties = GetScrollProperties();
    if (!ParseBoolean(value, properties.Enabled)) return false;
    SetScrollProperties(properties);
    return true;
  }
  if (name == "HorizontalScrollEnabled") {
    ScrollProperty properties = GetScrollProperties();
    if (!ParseBoolean(value, properties.Horizontal)) return false;
    SetScrollProperties(properties);
    return true;
  }
  if (name == "VerticalScrollEnabled") {
    ScrollProperty properties = GetScrollProperties();
    if (!ParseBoolean(value, properties.Vertical)) return false;
    SetScrollProperties(properties);
    return true;
  }
  if (name == "HorizontalScrollBar" || name == "VerticalScrollBar") {
    ScrollProperty properties = GetScrollProperties();
    auto& visibility = name == "HorizontalScrollBar"
        ? properties.HorizontalScrollBar : properties.VerticalScrollBar;
    if (!ParseScrollBarVisibility(value, visibility)) return false;
    SetScrollProperties(properties);
    return true;
  }
  if (name == "ShowHorizontalScrollBar") {
    bool visible = false;
    if (!ParseBoolean(value, visible)) return false;
    ScrollProperty properties = GetScrollProperties();
    properties.HorizontalScrollBar = visible
        ? ScrollBarVisibility::Visible
        : ScrollBarVisibility::Hidden;
    SetScrollProperties(properties);
    return true;
  }
  if (name == "ShowVerticalScrollBar") {
    bool visible = false;
    if (!ParseBoolean(value, visible)) return false;
    ScrollProperty properties = GetScrollProperties();
    properties.VerticalScrollBar = visible
        ? ScrollBarVisibility::Visible
        : ScrollBarVisibility::Hidden;
    SetScrollProperties(properties);
    return true;
  }
  if (name == "WheelStep") {
    ScrollProperty properties = GetScrollProperties();
    properties.WheelStep = std::max(1.0f,
                                    std::strtof(value.c_str(), nullptr));
    SetScrollProperties(properties);
    return true;
  }
  return RectNode::SetProperty(name, value);
}

bool PanelNode::OnMouseDown(MouseMovArgs args) {
  if (args.Button != MouseButton::Left || !Contains(args))
    return false;
  // 边界手势优先于标题栏拖拽，二者不会在同一次拖动中同时修改几何。
  if (BeginResize(this, args)) return true;
  const auto& properties = GetDragProperties();
  const bool inAllowedRegion = properties.Region == DragRegion::Anywhere ||
      static_cast<float>(args.Y) <= GetLayoutY() + TitleHeight;
  return BeginDrag(this, args, inAllowedRegion);
}

bool PanelNode::OnMouseDrag(MouseMovArgs args) {
  if (IsResizing()) return UpdateResize(this, args);
  return UpdateDrag(this, args);
}

bool PanelNode::OnMouseUp(MouseMovArgs) {
  const bool resized = EndResize();
  const bool dragged = EndDrag();
  return resized || dragged;
}

z8::MouseCursor PanelNode::GetMouseCursor(MouseMovArgs args) const {
  return GetResizeCursor(this, args);
}

bool PanelNode::OnMouseWheel(MouseWheelArgs args) {
  const auto& properties = GetScrollProperties();
  if (!properties.Enabled || !properties.Vertical ||
      !ScrollViewportNode->Contains(static_cast<float>(args.X),
                                    static_cast<float>(args.Y)) ||
      GetMaximumScrollOffsetY() <= 0.0f)
    return false;
  const float notches = static_cast<float>(args.Delta) /
                        static_cast<float>(WHEEL_DELTA);
  return ScrollVerticalBy(-notches * properties.WheelStep);
}

void PanelNode::ScrollOffsetChanged(float offset) {
  ScrollViewportNode->SetChildOffset(0.0f, -offset);
  VerticalScrollBarNode->SetValue(offset, false);
}

void PanelNode::OnLayoutUpdated() {
  float contentExtent = 0.0f;
  for (const auto& child : ContentNode->GetChildren()) {
    const auto node = child->GetYogaNode();
    contentExtent = (std::max)(contentExtent,
        YGNodeLayoutGetTop(node) + YGNodeLayoutGetHeight(node));
  }
  const float viewportHeight = ScrollViewportNode->GetLayoutHeight();
  SetVerticalScrollRange(viewportHeight, contentExtent);
  VerticalScrollBarNode->SetMetrics(viewportHeight,
                                    viewportHeight +
                                        GetMaximumScrollOffsetY());
  SetScrollOffsetY(GetScrollOffsetY());

  const auto& properties = GetScrollProperties();
  const auto visibility = properties.VerticalScrollBar;
  const bool showsScrollBar = properties.Enabled && properties.Vertical &&
      (visibility == ScrollBarVisibility::Visible ||
       (visibility == ScrollBarVisibility::Auto &&
        GetMaximumScrollOffsetY() > 0.0f));
  VerticalScrollBarNode->SetVisible(showsScrollBar);
  VerticalScrollThumbNode->SetVisible(showsScrollBar);
}

void PanelNode::ApplyScrollProperties() {
  const auto& properties = GetScrollProperties();
  const bool scrolls = properties.Enabled &&
                       (properties.Horizontal || properties.Vertical);
  // Yoga 负责按滚动容器语义测量溢出内容；实际偏移和滚动条视觉由后续
  // ScrollView 渲染阶段消费同一属性组，属性本身不与 Panel 交互代码耦合。
  YGNodeStyleSetOverflow(ScrollViewportNode->GetYogaNode(),
                         scrolls ? YGOverflowScroll : YGOverflowVisible);
  ScrollViewportNode->SetClipsChildren(scrolls);
  if (VerticalScrollBarNode) {
    VerticalScrollBarNode->SetVisible(false);
    VerticalScrollThumbNode->SetVisible(false);
  }
}
