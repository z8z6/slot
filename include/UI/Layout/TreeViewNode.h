#pragma once

#include "UI/Layout/ImageNode.h"
#include "UI/Layout/RectNode.h"
#include "UI/Layout/ScrollNode.h"
#include "UI/Layout/TextNode.h"

#include <functional>

namespace z8::ui {

class TreeViewNode;

/** TreeView 中拥有标题行和递归子项宿主的节点。 */
class TreeViewItemNode final : public RectNode {
private:
  TreeViewNode *Owner = nullptr;
  size_t Depth = 0;

public:
  RectNode *HeaderNode = nullptr;
  BaseNode *IndentNode = nullptr;
  ImageNode *ExpanderNode = nullptr;
  TextNode *LabelNode = nullptr;
  BaseNode *ItemsNode = nullptr;
  bool Expanded = true;
  bool Selected = false;
  bool Enabled = true;

  TreeViewItemNode();

  void Attach(TreeViewNode *owner, size_t depth);
  BaseNode *ContentHost() override { return ItemsNode; }
  const char *TypeName() const override { return "TreeItem"; }
  EventReply OnKeyDown(KeyArgs args) override;
  EventReply OnMouseDown(MouseMovArgs args) override;
  void SetEnabled(bool enabled);
  bool SetExpanded(bool expanded);
  bool SetProperty(const std::string &name, const std::string &value) override;
  void SetSelected(bool selected);
  void SetText(const std::string &text);

private:
  bool HasItems() const;
  void OnVisualStateChanged() override;
  void RefreshExpander();
};

/** 可滚动层级列表；拥有唯一选择项，但不接管 TreeItem 的业务数据生命周期。 */
class TreeViewNode : public ScrollNode {
public:
  TreeViewItemNode *SelectedItem = nullptr;
  std::function<void(TreeViewItemNode *)> SelectionChanged;

  TreeViewNode();

  const char *TypeName() const override { return "TreeView"; }
  void OnAfterLayout() override;
  bool SelectItem(TreeViewItemNode *item, bool notify = true);

private:
  void AttachItems(BaseNode &host, size_t depth,
                   TreeViewItemNode *&declaredSelection);
  void ClearSelection(BaseNode &host, TreeViewItemNode *except);
};

} // namespace z8::ui
