//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/BaseNode.h"

#include "Object/UIObject/UIObject.h"
#include "UI/Style/UITheme.h"
#include "yoga/YGNodeStyle.h"
#include "yoga/node/Node.h"

#include <algorithm>

using namespace z8::ui;

BaseNode::BaseNode() : Node(YGNodeNew()), UO(nullptr), Parent(nullptr){
  // 保存 BaseNode 指针到 YGNode, 这样可以双向查询
  YGNodeSetContext(Node, this);
  YGNodeStyleSetFlexGrow(Node, 1.0f);
  YGNodeStyleSetFlexShrink(Node, 1.0f);
}

BaseNode::~BaseNode() {
  // Behavior 可能观察内部视觉节点，必须在子树和 Yoga 几何失效前释放。
  CancelPointerCapture();
  for (const auto &behavior : Behaviors) {
    behavior->OnDetached();
    behavior->Owner = nullptr;
  }
  Behaviors.clear();
  YGNodeRemoveAllChildren(Node);
  Children.clear();
  UO.reset();
  YGNodeFree(Node);
  Node = nullptr;
}

bool BaseNode::Contains(float x, float y) const {
  return Visible && x >= LayoutX && y >= LayoutY &&
         x <= LayoutX + LayoutWidth && y <= LayoutY + LayoutHeight &&
         x >= VisibleClip.x && y >= VisibleClip.y && x <= VisibleClip.z &&
         y <= VisibleClip.w;
}
bool BaseNode::Contains(MouseMovArgs args) const {
  return Contains(static_cast<float>(args.X), static_cast<float>(args.Y));
}

UIBehavior *BaseNode::AddBehavior(std::unique_ptr<UIBehavior> behavior) {
  if (!behavior || behavior->Owner)
    return nullptr;
  auto *result = behavior.get();
  result->Owner = this;
  Behaviors.push_back(std::move(behavior));
  // stable_sort 让同优先级的声明顺序成为稳定且可预测的冲突规则。
  std::stable_sort(Behaviors.begin(), Behaviors.end(),
                   [](const auto &left, const auto &right) {
                     return left->GetPriority() > right->GetPriority();
                   });
  result->OnAttached();
  return result;
}

bool BaseNode::RemoveBehavior(UIBehavior *behavior) {
  if (!behavior)
    return false;
  const auto iterator = std::find_if(Behaviors.begin(), Behaviors.end(),
                                     [behavior](const auto &candidate) {
                                       return candidate.get() == behavior;
                                     });
  if (iterator == Behaviors.end())
    return false;
  if (CapturedBehavior == behavior) {
    behavior->OnCaptureLost();
    CapturedBehavior = nullptr;
  }
  (*iterator)->OnDetached();
  (*iterator)->Owner = nullptr;
  Behaviors.erase(iterator);
  return true;
}

UIEventReply BaseNode::DispatchMouseDown(MouseMovArgs args) {
  CapturedBehavior = nullptr;
  for (const auto &behavior : Behaviors) {
    const auto reply = behavior->OnMouseDown(args);
    if (reply == UIEventReply::Capture)
      CapturedBehavior = behavior.get();
    if (reply != UIEventReply::Ignored)
      return reply;
  }
  // 旧式复合控件仍可覆写节点钩子；true 按原契约解释为开始捕获。
  return OnMouseDown(args) ? UIEventReply::Capture : UIEventReply::Ignored;
}

bool BaseNode::DispatchMouseDrag(MouseMovArgs args) {
  return CapturedBehavior ? CapturedBehavior->OnMouseDrag(args)
                          : OnMouseDrag(args);
}

bool BaseNode::DispatchMouseUp(MouseMovArgs args) {
  if (!CapturedBehavior)
    return OnMouseUp(args);
  auto *behavior = CapturedBehavior;
  const bool handled = behavior->OnMouseUp(args);
  CapturedBehavior = nullptr;
  return handled;
}

bool BaseNode::DispatchMouseWheel(MouseWheelArgs args) {
  for (const auto &behavior : Behaviors)
    if (behavior->OnMouseWheel(args))
      return true;
  return OnMouseWheel(args);
}

z8::MouseCursor BaseNode::QueryMouseCursor(MouseMovArgs args) const {
  if (CapturedBehavior) {
    const auto cursor = CapturedBehavior->GetMouseCursor(args);
    if (cursor != MouseCursor::Arrow)
      return cursor;
  }
  for (const auto &behavior : Behaviors) {
    if (behavior.get() == CapturedBehavior)
      continue;
    const auto cursor = behavior->GetMouseCursor(args);
    if (cursor != MouseCursor::Arrow)
      return cursor;
  }
  return GetMouseCursor(args);
}

void BaseNode::DispatchLayoutUpdated() {
  for (const auto &behavior : Behaviors)
    behavior->OnLayoutUpdated();
  OnLayoutUpdated();
}

void BaseNode::CancelPointerCapture() {
  if (CapturedBehavior) {
    CapturedBehavior->OnCaptureLost();
    CapturedBehavior = nullptr;
  }
  OnPointerCaptureLost();
}

BaseNode *BaseNode::AddChild(std::unique_ptr<BaseNode> child) {
  if (!child)
    return nullptr;
  child->Parent = this;
  auto *result = child.get();
  YGNodeInsertChild(Node, child->GetYogaNode(), GetChildSize());
  Children.push_back(std::move(child));
  return result;
}

void BaseNode::RemoveChildrenFrom(size_t first) {
  if (first >= Children.size())
    return;
  for (size_t i = Children.size(); i > first; --i)
    YGNodeRemoveChild(Node, Children[i - 1]->GetYogaNode());
  Children.erase(Children.begin() + static_cast<std::ptrdiff_t>(first),
                   Children.end());
}

BaseNode *BaseNode::ContentHost() { return this; }

const char *BaseNode::TypeName() const { return "UI"; }

bool BaseNode::SetProperty(const std::string &name, const std::string &value) {
  // 声明层无需知道能力属于控件还是外挂 Behavior；挂载即自动进入属性链。
  for (const auto &behavior : Behaviors)
    if (behavior->SetProperty(name, value))
      return true;
  const float number = std::strtof(value.c_str(), nullptr);
  if (name == "Id" || name == "Key" || name == "Name")
    Key = value;
  else if (name == "Color") {
    DirectX::XMFLOAT4 color;
    if (!ParseUIColor(value, color))
      return false;
    return SetColor(color);
  } else if (name == "Width")
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

bool BaseNode::SetColor(const DirectX::XMFLOAT4 &color) {
  if (!UO)
    return false;
  UO->SetColor(color);
  return true;
}
