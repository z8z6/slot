//
// Created by zhou_zhengming on 2026/5/19.
//

#pragma once
#include "Application.h"
#include "Core/SceneObjectDetailsBinding.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace z8::ui {
class TreeViewItemNode;
}

namespace z8 {
class DemoApplication : public Application {
  std::unique_ptr<SceneObjectDetailsBinding> DetailsBinding;
  std::unordered_map<ui::TreeViewItemNode *, GameObject *> ItemObjects;
  std::unordered_map<GameObject *, ui::TreeViewItemNode *> ObjectItems;

  void BindEditorLayout();
  std::string DescribeObject(GameObject &object, size_t index) const;
  void OnFrame() override;
  void OnLayoutReloaded() override;
  void OnSceneSelectionChanged(GameObject *object) override;
  void BeforeInit() override;
};
} // namespace z8
