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
  if (distances[nearest] > Properties.EdgeThreshold)
    return;

  // 角落按最近边决定方向，避免固定 if 顺序使水平停靠永久压过垂直停靠。
  constexpr DockPlacement placements[] = {
      DockPlacement::Left, DockPlacement::Right, DockPlacement::Top,
      DockPlacement::Bottom};
  Properties.Placement = placements[nearest];
}
