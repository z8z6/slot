#include "UI/Behavior/DockLayoutBehavior.h"

#include "UI/Behavior/DockBehavior.h"
#include "UI/Layout/BaseNode.h"
#include "yoga/YGNodeStyle.h"

#include <algorithm>
#include <vector>

using namespace z8::ui;

void DockLayoutBehavior::ApplyGeometry(BaseNode *node, float left, float top,
                                       float width, float height) {
  if (!node)
    return;
  // 停靠框已经包含完整父空间坐标，必须清除主题 Margin，避免 Yoga 二次收缩。
  YGNodeStyleSetMargin(node->Node, YGEdgeAll, 0.0f);
  YGNodeStyleSetPositionType(node->Node, YGPositionTypeAbsolute);
  YGNodeStyleSetPosition(node->Node, YGEdgeLeft, left);
  YGNodeStyleSetPosition(node->Node, YGEdgeTop, top);
  YGNodeStyleSetWidth(node->Node, (std::max)(0.0f, width));
  YGNodeStyleSetHeight(node->Node, (std::max)(0.0f, height));
  YGNodeStyleSetFlexGrow(node->Node, 0.0f);
  YGNodeStyleSetFlexShrink(node->Node, 0.0f);
}

void DockLayoutBehavior::OnBeforeLayout(float width, float height) {
  auto *owner = GetOwner();
  if (!owner)
    return;

  float left = 0.0f;
  float top = 0.0f;
  float right = (std::max)(0.0f, width);
  float bottom = (std::max)(0.0f, height);
  std::vector<BaseNode *> flexible;

  for (const auto &child : owner->Children) {
    auto *dock = child->GetBehavior<DockBehavior>();
    if (!dock || !dock->Properties.Enabled ||
        dock->Properties.Placement == DockPlacement::Floating)
      continue;
    const float extent = (std::max)(1.0f, dock->Properties.Extent);
    switch (dock->Properties.Placement) {
    case DockPlacement::Left: {
      const float allocated = (std::min)(extent, right - left);
      ApplyGeometry(child.get(), left, top, allocated, bottom - top);
      left += allocated;
      break;
    }
    case DockPlacement::Right: {
      const float allocated = (std::min)(extent, right - left);
      right -= allocated;
      ApplyGeometry(child.get(), right, top, allocated, bottom - top);
      break;
    }
    case DockPlacement::Top: {
      const float allocated = (std::min)(extent, bottom - top);
      ApplyGeometry(child.get(), left, top, right - left, allocated);
      top += allocated;
      break;
    }
    case DockPlacement::Bottom: {
      const float allocated = (std::min)(extent, bottom - top);
      bottom -= allocated;
      ApplyGeometry(child.get(), left, bottom, right - left, allocated);
      break;
    }
    case DockPlacement::Auto:
    case DockPlacement::Fill:
      flexible.push_back(child.get());
      break;
    case DockPlacement::Floating:
      break;
    }
  }

  if (flexible.empty())
    return;
  // 多个自由 Panel 横向均分剩余矩形；最后一项吸收浮点余量，避免像素缝隙。
  const float totalWidth = (std::max)(0.0f, right - left);
  const float itemWidth = totalWidth / static_cast<float>(flexible.size());
  for (size_t i = 0; i < flexible.size(); ++i) {
    const float itemLeft = left + itemWidth * static_cast<float>(i);
    const float itemRight =
        i + 1 == flexible.size() ? right : itemLeft + itemWidth;
    ApplyGeometry(flexible[i], itemLeft, top, itemRight - itemLeft,
                  bottom - top);
  }
}
