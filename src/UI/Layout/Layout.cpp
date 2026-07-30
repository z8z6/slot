//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/Layout.h"

#include "Core/Application.h"
#include "UI/Layout/RectNode.h"
#include "UI/Object/UIObject/RectUIObject.h"
#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"

using namespace z8::ui;

Layout::Layout(Application *App) : App(App) {
  Add();
  Root = Nodes[0];
}

void Layout::Add(BaseNode *Node) {
  Nodes.push_back(Node);
  UOs.push_back(Node->O);
}

void Layout::Update() {
  auto w = static_cast<float>(App->Window.Width);
  auto h = static_cast<float>(App->Window.Height);
  // 计算整体布局
  YGNodeCalculateLayout(Root->Node, w, h, YGDirectionLTR);
  // 更新节点树
  UpdateTree(Root->Node, 0, 0);

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
  N->O->SetPosition(absX, absY);
  N->O->SetScale(width, height);

  for (size_t i = 0; i < N->GetChildCount(); ++i) {
    YGNodeRef child = YGNodeGetChild(Node, i);
    UpdateTree(child, absX, absY);
  }
}