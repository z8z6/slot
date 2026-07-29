//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/BaseNode.h"
#include "yoga/node/Node.h"

using namespace z8::ui;

BaseNode::BaseNode() : Node(YGNodeNew()), O(nullptr) {}
BaseNode::~BaseNode() {}