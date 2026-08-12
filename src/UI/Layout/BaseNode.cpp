//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/BaseNode.h"

#include "yoga/YGNodeStyle.h"
#include "yoga/node/Node.h"

#include <cstdlib>

using namespace z8::ui;

BaseNode::BaseNode() : Node(YGNodeNew()) {
  // 保存 BaseNode 指针到 YGNode, 这样可以双向查询
  YGNodeSetContext(Node, this);
  YGNodeStyleSetFlexGrow(Node, 1.0f);
  YGNodeStyleSetFlexShrink(Node, 1.0f);
}

BaseNode::~BaseNode() {
  YGNodeRemoveAllChildren(Node);
  Children.clear();
  YGNodeFree(Node);
  Node = nullptr;
}

bool BaseNode::Contains(float x, float y) const {
  return Visible && x >= Left && y >= Top && x <= Left + Width &&
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
  YGNodeInsertChild(Node, child->Node, Children.size());
  Children.push_back(std::move(child));
  return result;
}

void BaseNode::RemoveChildrenFrom(size_t first) {
  if (first >= Children.size())
    return;
  for (size_t i = Children.size(); i > first; --i)
    YGNodeRemoveChild(Node, Children[i - 1]->Node);
  Children.erase(Children.begin() + static_cast<std::ptrdiff_t>(first),
                 Children.end());
}

BaseNode *BaseNode::ContentHost() { return this; }

const char *BaseNode::TypeName() const { return "UI"; }

bool BaseNode::SetProperty(const std::string &name, const std::string &value) {
  const float number = std::strtof(value.c_str(), nullptr);
  if (name == "Id" || name == "Key" || name == "Name")
    Key = value;
  else if (name == "Width")
    YGNodeStyleSetWidth(Node, number);
  else if (name == "Height")
    YGNodeStyleSetHeight(Node, number);
  else if (name == "MinWidth")
    YGNodeStyleSetMinWidth(Node, number);
  else if (name == "MinHeight")
    YGNodeStyleSetMinHeight(Node, number);
  else if (name == "MaxWidth")
    YGNodeStyleSetMaxWidth(Node, number);
  else if (name == "MaxHeight")
    YGNodeStyleSetMaxHeight(Node, number);
  else if (name == "FlexGrow")
    YGNodeStyleSetFlexGrow(Node, number);
  else if (name == "FlexShrink")
    YGNodeStyleSetFlexShrink(Node, number);
  else if (name == "Margin")
    YGNodeStyleSetMargin(Node, YGEdgeAll, number);
  else if (name == "Padding")
    YGNodeStyleSetPadding(Node, YGEdgeAll, number);
  else if (name == "Direction") {
    if (value == "Row")
      YGNodeStyleSetFlexDirection(Node, YGFlexDirectionRow);
    else if (value == "Column")
      YGNodeStyleSetFlexDirection(Node, YGFlexDirectionColumn);
    else
      return false;
  } else
    return false;
  return true;
}
