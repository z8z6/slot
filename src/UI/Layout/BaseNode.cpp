//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/BaseNode.h"

#include "Object/UIObject/UIObject.h"
#include "UI/Style/UITheme.h"
#include "yoga/node/Node.h"
#include "yoga/YGNodeStyle.h"

using namespace z8::ui;

BaseNode::BaseNode() : YogaNode(YGNodeNew()) {
  // 保存 BaseNode 指针到 YGNode, 这样可以双向查询
  YGNodeSetContext(YogaNode, this);
  // UI 默认采用 flex 布局
  YGNodeStyleSetFlexGrow(YogaNode, 1.0f);
  YGNodeStyleSetFlexShrink(YogaNode, 1.0f);
}

BaseNode::~BaseNode() {
  // Yoga 不拥有子节点；先断开 Yoga 关系，再由 unique_ptr 递归释放控件树。
  YGNodeRemoveAllChildren(YogaNode);
  ChildNodes.clear();
  UO.reset();
  YGNodeFree(YogaNode);
  YogaNode = nullptr;
}

void BaseNode::SetObject(std::unique_ptr<UIObject> object) {
  UO = std::move(object);
}

size_t BaseNode::GetChildCount() const {
  return ChildNodes.size();
}

BaseNode* BaseNode::GetChild(size_t index) const {
  return index < ChildNodes.size() ? ChildNodes[index].get() : nullptr;
}

bool BaseNode::Contains(float x, float y) const {
  return Visible && x >= LayoutX && y >= LayoutY &&
         x <= LayoutX + LayoutWidth && y <= LayoutY + LayoutHeight &&
         x >= VisibleClip.x && y >= VisibleClip.y &&
         x <= VisibleClip.z && y <= VisibleClip.w;
}
bool BaseNode::Contains(MouseMovArgs args) const {
  return Contains(static_cast<float>(args.X),static_cast<float>(args.Y));
}

BaseNode * BaseNode::AddChild(std::unique_ptr<BaseNode> child) {
  if (!child) return nullptr;
  child->Parent = this;
  auto* result = child.get();
  YGNodeInsertChild(YogaNode, child->GetYogaNode(), GetChildCount());
  ChildNodes.push_back(std::move(child));
  return result;
}

void BaseNode::RemoveChildrenFrom(size_t first) {
  if (first >= ChildNodes.size()) return;
  for (size_t i = ChildNodes.size(); i > first; --i)
    YGNodeRemoveChild(YogaNode, ChildNodes[i - 1]->GetYogaNode());
  ChildNodes.erase(ChildNodes.begin() + static_cast<std::ptrdiff_t>(first), ChildNodes.end());
}

BaseNode* BaseNode::ContentHost() { return this; }

const char* BaseNode::TypeName() const { return "UI"; }

bool BaseNode::SetProperty(const std::string& name, const std::string& value) {
  const float number = std::strtof(value.c_str(), nullptr);
  if (name == "Id" || name == "Key" || name == "Name") Key = value;
  else if (name == "Color") {
    DirectX::XMFLOAT4 color;
    if (!ParseUIColor(value, color)) return false;
    return SetColor(color);
  }
  else if (name == "Width") YGNodeStyleSetWidth(YogaNode, number);
  else if (name == "Height") YGNodeStyleSetHeight(YogaNode, number);
  else if (name == "MinWidth") YGNodeStyleSetMinWidth(YogaNode, number);
  else if (name == "MinHeight") YGNodeStyleSetMinHeight(YogaNode, number);
  else if (name == "MaxWidth") YGNodeStyleSetMaxWidth(YogaNode, number);
  else if (name == "MaxHeight") YGNodeStyleSetMaxHeight(YogaNode, number);
  else if (name == "FlexGrow") YGNodeStyleSetFlexGrow(YogaNode, number);
  else if (name == "FlexShrink") YGNodeStyleSetFlexShrink(YogaNode, number);
  else if (name == "Margin") YGNodeStyleSetMargin(YogaNode, YGEdgeAll, number);
  else if (name == "Padding") YGNodeStyleSetPadding(YogaNode, YGEdgeAll, number);
  else if (name == "Direction") {
    if (value == "Row") YGNodeStyleSetFlexDirection(YogaNode, YGFlexDirectionRow);
    else if (value == "Column") YGNodeStyleSetFlexDirection(YogaNode, YGFlexDirectionColumn);
    else return false;
  } else return false;
  return true;
}

bool BaseNode::SetColor(const DirectX::XMFLOAT4& color) {
  if (!UO) return false;
  UO->SetColor(color);
  return true;
}
