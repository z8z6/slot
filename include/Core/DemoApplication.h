//
// Created by zhou_zhengming on 2026/5/19.
//

#pragma once
#include "Application.h"

#include <string>
#include <unordered_map>

namespace z8::ui {
class TreeViewItemNode;
}

namespace z8 {
class DemoApplication : public Application {
  std::unordered_map<ui::TreeViewItemNode *, GameObject *> ItemObjects;
  std::unordered_map<GameObject *, ui::TreeViewItemNode *> ObjectItems;
  std::unordered_map<GameObject *, std::string> ObjectNames;

  void BindEditorLayout();
  std::string DescribeObject(GameObject &object, size_t index) const;
  void OnLayoutReloaded() override;
  void OnSceneSelectionChanged(GameObject *object) override;
  void PrepareScene() override;
  void UpdateDetails(GameObject *object);

public:
  void Init() override;
};
} // namespace z8
