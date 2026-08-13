#include "UI/Behavior/DockBehavior.h"

#include <algorithm>
#include <cstdlib>

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
  // DockWorkspace 持有唯一 DragSession；Behavior 只保留声明配置和身份能力。
}

void DockBehavior::OnDragCompleted(MouseMovArgs) {}
