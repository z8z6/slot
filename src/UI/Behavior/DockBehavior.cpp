#include "UI/Behavior/DockBehavior.h"
#include "UI/Property/PropertyParser.h"

#include <algorithm>

using namespace z8::ui;

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
    float threshold = 0.0f;
    if (!ParseFiniteFloat(value, threshold))
      return false;
    Properties.EdgeThreshold = (std::max)(0.0f, threshold);
    return true;
  }
  if (name == "DockExtent") {
    float extent = 0.0f;
    if (!ParseFiniteFloat(value, extent))
      return false;
    // 停靠尺寸参与 Split 比例求解，至少一个逻辑像素可避免退化叶节点。
    Properties.Extent = (std::max)(1.0f, extent);
    return true;
  }
  return false;
}

void DockBehavior::OnDragStarted(MouseMovArgs) {
  // DockWorkspace 持有唯一 DragSession；Behavior 只保留声明配置和身份能力。
}

void DockBehavior::OnDragCompleted(MouseMovArgs) {}
