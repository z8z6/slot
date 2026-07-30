//
// Created by zhou_zhengming on 2026/7/29.
//

#pragma once
#include "RectNode.h"

#include <vector>

namespace z8 {
class Application;
class GameObject;
}

namespace z8::ui {
class BaseNode;
class Layout {
public:
  Application* App;
  std::vector<BaseNode*> Nodes;
  std::vector<GameObject*> UOs;
  BaseNode* Root;

  explicit Layout(Application* App);
  void Add(BaseNode* Node = new RectNode);
  void Update();

private:
  void UpdateTree(YGNodeRef Node, float parentX, float parentY);
};
}





