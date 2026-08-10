//
// Created by zhou_zhengming on 2026/7/29.
//

#pragma once
#include "RectNode.h"

#include <memory>
#include <string>
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
  ~Layout();

  void SetRoot(std::unique_ptr<BaseNode> root);
  void RebuildIndex();
  BaseNode* Find(const std::string& key) const;
  void Update();
  void Calculate(float width, float height);

  void MarkTopologyDirty() { TopologyDirty = true; }
  bool ConsumeTopologyDirty();

private:
  std::unique_ptr<BaseNode> RootOwner;
  bool TopologyDirty = true;
  void IndexTree(BaseNode* node);
  void UpdateTree(YGNodeRef Node, float parentX, float parentY);
};
}





