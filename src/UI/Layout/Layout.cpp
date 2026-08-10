//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/Layout.h"

#include "UI/Layout/RectNode.h"
#include "UI/Object/UIObject/RectUIObject.h"
#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"

#include <iostream>
#include <ostream>

using namespace z8::ui;

Layout::Layout(Application *App) : App(App), Root(nullptr) {
  auto root = std::make_unique<BaseNode>();
  root->Key = "Root";
  SetRoot(std::move(root));
}

Layout::~Layout() = default;

void Layout::SetRoot(std::unique_ptr<BaseNode> root) {
  RootOwner = std::move(root);
  Root = RootOwner.get();
  RebuildIndex();
}

void Layout::IndexTree(BaseNode* node) {
  if (!node) return;
  Nodes.push_back(node);
  if (node->GetUO()) UOs.push_back(node->GetUO());
  for (const auto& child : node->GetChildren()) IndexTree(child.get());
}

void Layout::RebuildIndex() {
  Nodes.clear();
  UOs.clear();
  IndexTree(Root);
  TopologyDirty = true;
}

BaseNode* Layout::Find(const std::string& key) const {
  for (auto* node : Nodes) if (node->Key == key) return node;
  return nullptr;
}

bool Layout::ConsumeTopologyDirty() {
  const bool result = TopologyDirty;
  TopologyDirty = false;
  return result;
}

void Layout::Calculate(float w, float h) {
  // 计算整体布局
  YGNodeCalculateLayout(Root->GetYogaNode(), w, h, YGDirectionLTR);
  // 更新节点树
  UpdateTree(Root->GetYogaNode(), 0, 0);

}


void Layout::UpdateTree(YGNodeRef Node, float parentX, float parentY) {
  auto N = static_cast<BaseNode *>(YGNodeGetContext(Node));

  // 相对于父容器的位置
  float x = YGNodeLayoutGetLeft(Node);
  float y = YGNodeLayoutGetTop(Node);

  // 长宽
  float width = YGNodeLayoutGetWidth(Node);
  float height = YGNodeLayoutGetHeight(Node);

  // 计算绝对位置
  float absX = parentX + x;
  float absY = parentY + y;

  // 应用位置和大小
  if (N->GetUO()) {
    N->GetUO()->SetPosition(absX, absY, width, height);
    N->GetUO()->SetScale(width, height);
  }

  for (size_t i = 0; i < N->GetChildCount(); ++i) {
    YGNodeRef child = YGNodeGetChild(Node, i);
    UpdateTree(child, absX, absY);
  }
}
