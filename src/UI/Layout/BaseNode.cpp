//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/BaseNode.h"
#include "UI/Property/PropertyParser.h"

using namespace z8::ui;

BaseNode::BaseNode() = default;

BaseNode::~BaseNode() = default;

bool BaseNode::Contains(float x, float y) const {
  return EffectiveVisible && x >= Left && y >= Top && x <= Left + Width &&
         y <= Top + Height && x >= VisibleClip.x && y >= VisibleClip.y &&
         x <= VisibleClip.z && y <= VisibleClip.w;
}
bool BaseNode::Contains(MouseMovArgs args) const {
  return Contains(static_cast<float>(args.X), static_cast<float>(args.Y));
}

void BaseNode::DispatchAfterLayout() {
  OnAfterLayout();
}

BaseNode *BaseNode::AddChild(std::unique_ptr<BaseNode> child) {
  if (!child) return nullptr;
  child->Parent = this;
  auto *result = child.get();
  Children.push_back(std::move(child));
  return result;
}

void BaseNode::RemoveChildrenFrom(size_t first) {
  if (first >= Children.size())
    return;
  Children.erase(Children.begin() + static_cast<std::ptrdiff_t>(first),
                 Children.end());
}

BaseNode *BaseNode::ContentHost() { return this; }

const char *BaseNode::TypeName() const { return "UI"; }

bool BaseNode::SetProperty(const std::string &name, const std::string &value) {
  if (name == "Id" || name == "Key" || name == "Name") {
    Key = value;
    return true;
  }
  if (name == "Direction") {
    if (value == "Row")
      Style.Direction = FlexDirection::Row;
    else if (value == "Column")
      Style.Direction = FlexDirection::Column;
    else
      return false;
    return true;
  }

  // 通用布局数值必须完整、有限；先解析到局部值，失败时不污染已有布局状态。
  float number = 0.0f;
  if (!ParseFiniteFloat(value, number))
    return false;
  if (name == "Width")
    Style.Width = number;
  else if (name == "Height")
    Style.Height = number;
  else if (name == "MinWidth")
    Style.MinWidth = number;
  else if (name == "MinHeight")
    Style.MinHeight = number;
  else if (name == "MaxWidth")
    Style.MaxWidth = number;
  else if (name == "MaxHeight")
    Style.MaxHeight = number;
  else if (name == "FlexGrow")
    Style.FlexGrow = number;
  else if (name == "FlexShrink")
    Style.FlexShrink = number;
  else if (name == "Margin")
    Style.Margin = number;
  else if (name == "Padding")
    Style.Padding = number;
  else
    return false;
  return true;
}
