#include "UI/Behavior/DockLayoutBehavior.h"

#include "UI/Behavior/DockBehavior.h"
#include "UI/Layout/BaseNode.h"
#include "UI/Layout/BehaviorNode.h"
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
  auto *owner = Owner;
  if (!owner)
    return;

  float left = 0.0f;
  float top = 0.0f;
  float right = (std::max)(0.0f, width);
  float bottom = (std::max)(0.0f, height);
  std::vector<BaseNode *> flexible;

  size_t horizontalShares = 0;
  size_t verticalShares = 0;
  // 第一阶段先应用显式 Extent。固定工具栏/侧栏属于工作区边界，不参与用户
  // 停靠后的均分，否则任意一次交互都会改变应用预设的编辑器骨架。
  for (const auto &child : owner->Children) {
    auto *behaviorChild = dynamic_cast<BehaviorNode *>(child.get());
    auto *dock = behaviorChild
                     ? behaviorChild->GetBehavior<DockBehavior>()
                     : nullptr;
    if (!dock || !dock->Properties.Enabled ||
        dock->Properties.Placement == DockPlacement::Floating)
      continue;
    switch (dock->Properties.Placement) {
    case DockPlacement::Left:
    case DockPlacement::Right:
      if (dock->Properties.EqualShare)
        ++horizontalShares;
      break;
    case DockPlacement::Top:
    case DockPlacement::Bottom:
      if (dock->Properties.EqualShare)
        ++verticalShares;
      break;
    case DockPlacement::Auto:
    case DockPlacement::Fill:
      // Fill 是对应轴最后一个槽位；把它计入分母才能让新停靠 Panel 与原有
      // 同级 Panel 等宽/等高，而不是让边缘项沿用浮动窗口旧尺寸。
      ++horizontalShares;
      ++verticalShares;
      break;
    case DockPlacement::Floating:
      break;
    }
  }

  for (const auto &child : owner->Children) {
    auto *behaviorChild = dynamic_cast<BehaviorNode *>(child.get());
    auto *dock = behaviorChild
                     ? behaviorChild->GetBehavior<DockBehavior>()
                     : nullptr;
    if (!dock || !dock->Properties.Enabled ||
        dock->Properties.Placement == DockPlacement::Floating)
      continue;
    if (dock->Properties.EqualShare) continue;
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

  // 第二阶段在固定边界留下的矩形内分配交互槽。每次按剩余槽数切分，保证
  // 同级目标、多个已停靠 Panel 与 Fill 内容最终得到相同轴向尺寸。
  for (const auto &child : owner->Children) {
    auto *behaviorChild = dynamic_cast<BehaviorNode *>(child.get());
    auto *dock = behaviorChild
                     ? behaviorChild->GetBehavior<DockBehavior>()
                     : nullptr;
    if (!dock || !dock->Properties.Enabled || !dock->Properties.EqualShare ||
        dock->Properties.Placement == DockPlacement::Floating)
      continue;
    const bool horizontal = dock->Properties.Placement == DockPlacement::Left ||
                            dock->Properties.Placement == DockPlacement::Right;
    size_t &shares = horizontal ? horizontalShares : verticalShares;
    const float available = horizontal ? right - left : bottom - top;
    const float extent = shares > 0
                             ? available / static_cast<float>(shares)
                             : available;
    if (shares > 0) --shares;
    switch (dock->Properties.Placement) {
    case DockPlacement::Left:
      ApplyGeometry(child.get(), left, top, extent, bottom - top);
      left += extent;
      break;
    case DockPlacement::Right:
      right -= extent;
      ApplyGeometry(child.get(), right, top, extent, bottom - top);
      break;
    case DockPlacement::Top:
      ApplyGeometry(child.get(), left, top, right - left, extent);
      top += extent;
      break;
    case DockPlacement::Bottom:
      bottom -= extent;
      ApplyGeometry(child.get(), left, bottom, right - left, extent);
      break;
    case DockPlacement::Auto:
    case DockPlacement::Fill:
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
