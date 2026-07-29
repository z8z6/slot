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
  float w = YGNodeLayoutGetWidth(Root->Node);
  float h = YGNodeLayoutGetHeight(Root->Node);
  YGNodeCalculateLayout(Root->Node, w, h, YGDirectionLTR);
}

void Layout::Resize() {
  float w = static_cast<float>(App->Window.Width);
  float h = static_cast<float>(App->Window.Height);
  YGNodeStyleSetWidth(Root->Node, w);
  YGNodeStyleSetHeight(Root->Node, h);
  Update();
}