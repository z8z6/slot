#include "UI/Behavior/DockBehavior.h"

#include "UI/Layout/BaseNode.h"
#include "UI/Layout/BehaviorNode.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>

using namespace z8::ui;

namespace {

bool ParseBoolean(const std::string &value, bool &result) {
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

} // namespace

bool DockBehavior::SetProperty(const std::string &name,
                               const std::string &value) {
  if (name == "DockEnabled")
    return ParseBoolean(value, Properties.Enabled);
  if (name == "Dock") {
    if (value == "Auto")
      Properties.Placement = DockPlacement::Auto;
    else if (value == "Floating")
      Properties.Placement = DockPlacement::Floating;
    else if (value == "Left")
      Properties.Placement = DockPlacement::Left;
    else if (value == "Right")
      Properties.Placement = DockPlacement::Right;
    else if (value == "Top")
      Properties.Placement = DockPlacement::Top;
    else if (value == "Bottom")
      Properties.Placement = DockPlacement::Bottom;
    else if (value == "Fill")
      Properties.Placement = DockPlacement::Fill;
    else
      return false;
    return true;
  }
  if (name == "DockThreshold") {
    Properties.EdgeThreshold =
        (std::max)(0.0f, std::strtof(value.c_str(), nullptr));
    return true;
  }
  if (name == "DockExtent") {
    Properties.Extent =
        (std::max)(1.0f, std::strtof(value.c_str(), nullptr));
    return true;
  }
  return false;
}

void DockBehavior::OnDragStarted(MouseMovArgs) {
  if (!Properties.Enabled)
    return;
  // DockLayout 会跳过 Floating 节点，后续帧才不会覆盖 DragBehavior 写入的几何。
  Properties.Placement = DockPlacement::Floating;
}

void DockBehavior::OnDragCompleted(MouseMovArgs args) {
  if (!Properties.Enabled)
    return;
  const auto *owner = Owner;
  const auto *parent = owner ? owner->Parent : nullptr;
  if (!parent)
    return;

  const float x = static_cast<float>(args.X);
  const float y = static_cast<float>(args.Y);
  const float distances[] = {std::abs(x - parent->Left),
                             std::abs(x - (parent->Left + parent->Width)),
                             std::abs(y - parent->Top),
                             std::abs(y - (parent->Top + parent->Height))};
  const auto nearest =
      static_cast<size_t>(std::min_element(std::begin(distances),
                                           std::end(distances)) -
                          std::begin(distances));
  constexpr DockPlacement placements[] = {
      DockPlacement::Left, DockPlacement::Right, DockPlacement::Top,
      DockPlacement::Bottom};
  if (distances[nearest] <= Properties.EdgeThreshold) {
    // 根工作区边缘优先，保证拖到窗口外沿的传统停靠手势不被下方兄弟吸附。
    Properties.Placement = placements[nearest];
    Properties.EqualShare = true;
    return;
  }

  // 内部落点再解析兄弟 Panel。它们共享父 DockSpace，因此把相对边转换成同方向
  // 的停靠槽后，DockLayout 会在下一帧统一重排所有兄弟，避免几何互相覆盖。
  for (const auto &sibling : parent->Children) {
    if (sibling.get() == owner || std::string_view(sibling->TypeName()) != "Panel" ||
        !sibling->Contains(x, y))
      continue;
    const float halfWidth = (std::max)(1.0f, sibling->Width * 0.5f);
    const float halfHeight = (std::max)(1.0f, sibling->Height * 0.5f);
    const float horizontal =
        (x - (sibling->Left + halfWidth)) / halfWidth;
    const float vertical =
        (y - (sibling->Top + halfHeight)) / halfHeight;
    // 使用相对中心的归一化方向划分四个停靠区；宽面板不会因为上下边更近而
    // 吞掉左右区域，用户拖到左半/右半时能稳定得到水平停靠反馈。
    size_t siblingEdge = 0;
    if (std::abs(horizontal) >= std::abs(vertical))
      siblingEdge = horizontal < 0.0f ? 0U : 1U;
    else
      siblingEdge = vertical < 0.0f ? 2U : 3U;
    Properties.Placement = placements[siblingEdge];
    Properties.Extent = siblingEdge < 2 ? (std::max)(1.0f, owner->Width)
                                        : (std::max)(1.0f, owner->Height);
    // 用户停靠表达的是把当前同级工作区切成槽位，而不是用浮动窗口旧尺寸
    // 覆盖目标；布局阶段会让同轴参与者共享可用长度。
    Properties.EqualShare = true;
    return;
  }

}
