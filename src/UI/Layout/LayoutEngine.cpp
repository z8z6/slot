#include "UI/Layout/LayoutEngine.h"

#include "UI/Layout/BaseNode.h"

#include <algorithm>
#include <cmath>
#include <vector>

using namespace z8::ui;

namespace {

float Clamp(float value, float minimum, float maximum) {
  return std::clamp((std::max)(0.0f, value), minimum, maximum);
}

float ResolveWidth(const LayoutStyle &style, float available) {
  if (style.Width)
    return Clamp(*style.Width, style.MinWidth, style.MaxWidth);
  if (style.WidthPercent)
    return Clamp(available * *style.WidthPercent / 100.0f, style.MinWidth,
                 style.MaxWidth);
  return Clamp(available, style.MinWidth, style.MaxWidth);
}

float IntrinsicWidth(const BaseNode &node) {
  const auto &style = node.Style;
  if (style.Width)
    return Clamp(*style.Width, style.MinWidth, style.MaxWidth);
  float extent = 0.0f;
  for (const auto &child : node.Children) {
    if (child->Style.Position == PositionType::Absolute)
      continue;
    const float outer = IntrinsicWidth(*child) + child->Style.Margin * 2.0f;
    extent = style.Direction == FlexDirection::Row ? extent + outer
                                                    : (std::max)(extent, outer);
  }
  return Clamp(extent + style.Padding * 2.0f, style.MinWidth, style.MaxWidth);
}

float IntrinsicHeight(const BaseNode &node) {
  const auto &style = node.Style;
  if (style.Height)
    return Clamp(*style.Height, style.MinHeight, style.MaxHeight);
  float extent = 0.0f;
  for (const auto &child : node.Children) {
    if (child->Style.Position == PositionType::Absolute)
      continue;
    const float outer = IntrinsicHeight(*child) + child->Style.Margin * 2.0f;
    extent = style.Direction == FlexDirection::Column ? extent + outer
                                                       : (std::max)(extent, outer);
  }
  return Clamp(extent + style.Padding * 2.0f, style.MinHeight, style.MaxHeight);
}

void Arrange(BaseNode &node, float width, float height) {
  node.Computed.Width = Clamp(width, node.Style.MinWidth, node.Style.MaxWidth);
  node.Computed.Height =
      Clamp(height, node.Style.MinHeight, node.Style.MaxHeight);

  const bool row = node.Style.Direction == FlexDirection::Row;
  const float padding = node.Style.Padding;
  const float innerWidth = (std::max)(0.0f, node.Computed.Width - padding * 2.0f);
  const float innerHeight =
      (std::max)(0.0f, node.Computed.Height - padding * 2.0f);
  const float availableMain = row ? innerWidth : innerHeight;
  const float availableCross = row ? innerHeight : innerWidth;

  struct FlowItem {
    BaseNode *Node;
    float Basis;
    float Main;
  };
  std::vector<FlowItem> flow;
  float occupied = 0.0f;
  float totalGrow = 0.0f;
  float totalShrinkWeight = 0.0f;
  for (const auto &childOwner : node.Children) {
    auto &child = *childOwner;
    if (child.Style.Position == PositionType::Absolute)
      continue;
    const float basis = row ? IntrinsicWidth(child) : IntrinsicHeight(child);
    flow.push_back({&child, basis, basis});
    occupied += basis + child.Style.Margin * 2.0f;
    totalGrow += (std::max)(0.0f, child.Style.FlexGrow);
    totalShrinkWeight +=
        (std::max)(0.0f, child.Style.FlexShrink) * (std::max)(1.0f, basis);
  }

  const float freeSpace = availableMain - occupied;
  for (auto &item : flow) {
    if (freeSpace > 0.0f && totalGrow > 0.0f)
      item.Main += freeSpace * item.Node->Style.FlexGrow / totalGrow;
    else if (freeSpace < 0.0f && totalShrinkWeight > 0.0f) {
      const float weight = item.Node->Style.FlexShrink *
                           (std::max)(1.0f, item.Basis);
      item.Main += freeSpace * weight / totalShrinkWeight;
    }
    const auto &childStyle = item.Node->Style;
    item.Main = row ? Clamp(item.Main, childStyle.MinWidth, childStyle.MaxWidth)
                    : Clamp(item.Main, childStyle.MinHeight,
                            childStyle.MaxHeight);
  }

  // 某个项目命中 min/max 后，它未能吸收的空间必须继续分给其余项目。单次按
  // 比例夹紧会让总宽高越过容器；逐轮冻结边界项目可保持 Flex 总量守恒。
  for (size_t pass = 0; pass < flow.size(); ++pass) {
    float resolved = 0.0f;
    for (const auto &item : flow)
      resolved += item.Main + item.Node->Style.Margin * 2.0f;
    const float remainder = availableMain - resolved;
    if (std::abs(remainder) < 0.001f)
      break;
    float weightSum = 0.0f;
    for (const auto &item : flow) {
      const auto &style = item.Node->Style;
      const float minimum = row ? style.MinWidth : style.MinHeight;
      const float maximum = row ? style.MaxWidth : style.MaxHeight;
      if (remainder > 0.0f && item.Main < maximum)
        weightSum += (std::max)(0.0f, style.FlexGrow);
      else if (remainder < 0.0f && item.Main > minimum)
        weightSum += (std::max)(0.0f, style.FlexShrink) *
                     (std::max)(1.0f, item.Basis);
    }
    if (weightSum <= 0.0f)
      break;
    for (auto &item : flow) {
      const auto &style = item.Node->Style;
      const float minimum = row ? style.MinWidth : style.MinHeight;
      const float maximum = row ? style.MaxWidth : style.MaxHeight;
      float weight = 0.0f;
      if (remainder > 0.0f && item.Main < maximum)
        weight = (std::max)(0.0f, style.FlexGrow);
      else if (remainder < 0.0f && item.Main > minimum)
        weight = (std::max)(0.0f, style.FlexShrink) *
                 (std::max)(1.0f, item.Basis);
      item.Main = Clamp(item.Main + remainder * weight / weightSum, minimum,
                        maximum);
    }
  }

  float cursor = padding;
  for (auto &item : flow) {
    auto &child = *item.Node;
    const float margin = child.Style.Margin;
    cursor += margin;
    float childWidth = row ? item.Main
                           : ResolveWidth(child.Style,
                                          (std::max)(0.0f, availableCross -
                                                               margin * 2.0f));
    float childHeight = row
                            ? (child.Style.Height
                                   ? Clamp(*child.Style.Height,
                                           child.Style.MinHeight,
                                           child.Style.MaxHeight)
                                   : Clamp((std::max)(0.0f, availableCross -
                                                              margin * 2.0f),
                                           child.Style.MinHeight,
                                           child.Style.MaxHeight))
                            : item.Main;
    child.Computed.Left = row ? cursor : padding + margin;
    child.Computed.Top = row ? padding + margin : cursor;
    Arrange(child, childWidth, childHeight);
    cursor += item.Main + margin;
  }

  for (const auto &childOwner : node.Children) {
    auto &child = *childOwner;
    if (child.Style.Position != PositionType::Absolute)
      continue;
    const auto &style = child.Style;
    float childWidth = style.Width
                           ? *style.Width
                           : style.Left && style.Right
                                 ? innerWidth - *style.Left - *style.Right
                                 : IntrinsicWidth(child);
    float childHeight = style.Height
                            ? *style.Height
                            : style.Top && style.Bottom
                                  ? innerHeight - *style.Top - *style.Bottom
                                  : IntrinsicHeight(child);
    childWidth = Clamp(childWidth, style.MinWidth, style.MaxWidth);
    childHeight = Clamp(childHeight, style.MinHeight, style.MaxHeight);
    child.Computed.Left = padding +
                          (style.Left ? *style.Left
                                      : style.Right
                                            ? innerWidth - *style.Right - childWidth
                                            : 0.0f);
    child.Computed.Top = padding +
                         (style.Top ? *style.Top
                                    : style.Bottom
                                          ? innerHeight - *style.Bottom - childHeight
                                          : 0.0f);
    Arrange(child, childWidth, childHeight);
  }
}

} // namespace

void LayoutEngine::Calculate(BaseNode &root, float width, float height) {
  root.Computed = {0.0f, 0.0f, (std::max)(0.0f, width),
                   (std::max)(0.0f, height)};
  // 根尺寸来自窗口客户区，不能被声明样式收缩；否则布局坐标与交换链尺寸分离。
  Arrange(root, root.Computed.Width, root.Computed.Height);
}
