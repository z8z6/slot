//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/BaseNode.h"

#include "UI/Object/UIObject/UIObject.h"
#include "yoga/node/Node.h"

using namespace z8::ui;

BaseNode::BaseNode() : Node(YGNodeNew()), O(nullptr) {
  // 保存 Node 指针到 YGNode
  YGNodeSetContext(Node, this);
  // 默认 flex 布局
  YGNodeStyleSetFlexGrow(Node, 1.0f);
  YGNodeStyleSetFlexShrink(Node, 1.0f);
}

BaseNode::~BaseNode() = default;

size_t BaseNode::GetChildCount() const {
  return YGNodeGetChildCount(Node);
}

void BaseNode::AddChild(BaseNode *child) const {
  YGNodeInsertChild(Node, child->Node, GetChildCount());
}
