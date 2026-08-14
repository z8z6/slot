#include "UI/Layout/Layout.h"
#include "UI/Layout/TreeViewNode.h"
#include "UI/Style/Theme.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(TreeViewNodeTest, AttachesNestedItemsAndMaintainsUniqueSelection) {
  Layout layout;
  auto tree = std::make_unique<TreeViewNode>();
  auto *treeObserver = tree.get();
  tree->Style.Width = 240.0f;
  tree->Style.Height = 160.0f;

  auto parent = std::make_unique<TreeViewItemNode>();
  auto *parentObserver = parent.get();
  parent->SetText("Root");
  auto child = std::make_unique<TreeViewItemNode>();
  auto *childObserver = child.get();
  child->SetText("Child");
  parent->ContentHost()->AddChild(std::move(child));
  tree->ContentHost()->AddChild(std::move(parent));
  layout.Root->AddChild(std::move(tree));
  layout.RebuildIndex();

  // 第一次布局建立层级归属，第二次把深度缩进送入布局求解。
  layout.Calculate(320.0f, 240.0f);
  layout.Calculate(320.0f, 240.0f);
  ASSERT_TRUE(parentObserver->ExpanderNode->Visible);
  EXPECT_FLOAT_EQ(childObserver->IndentNode->Style.Width.value(),
                  Theme::Default().TreeView.Indent);

  EXPECT_TRUE(treeObserver->SelectItem(parentObserver));
  EXPECT_TRUE(parentObserver->Selected);
  EXPECT_TRUE(treeObserver->SelectItem(childObserver));
  EXPECT_FALSE(parentObserver->Selected);
  EXPECT_TRUE(childObserver->Selected);
  EXPECT_EQ(treeObserver->SelectedItem, childObserver);
}

TEST(TreeViewNodeTest, CollapseRemovesDescendantsFromLayoutAndChangesIcon) {
  Layout layout;
  auto tree = std::make_unique<TreeViewNode>();
  auto *treeObserver = tree.get();
  auto parent = std::make_unique<TreeViewItemNode>();
  auto *parentObserver = parent.get();
  auto child = std::make_unique<TreeViewItemNode>();
  auto *childObserver = child.get();
  parent->ContentHost()->AddChild(std::move(child));
  tree->ContentHost()->AddChild(std::move(parent));
  layout.Root->AddChild(std::move(tree));
  layout.RebuildIndex();
  layout.Calculate(320.0f, 240.0f);

  ASSERT_TRUE(parentObserver->SetExpanded(false));
  layout.Calculate(320.0f, 240.0f);
  EXPECT_FALSE(parentObserver->ItemsNode->Visible);
  ASSERT_TRUE(parentObserver->ItemsNode->Style.Height.has_value());
  EXPECT_FLOAT_EQ(parentObserver->ItemsNode->Style.Height.value(), 0.0f);
  EXPECT_FALSE(childObserver->EffectiveVisible);
  EXPECT_EQ(parentObserver->ExpanderNode->Icon, UIIcon::ChevronRight);

  EXPECT_TRUE(parentObserver->SetExpanded(true));
  layout.Calculate(320.0f, 240.0f);
  EXPECT_TRUE(childObserver->EffectiveVisible);
  EXPECT_EQ(parentObserver->ExpanderNode->Icon, UIIcon::ChevronDown);
  EXPECT_EQ(treeObserver->SelectedItem, nullptr);
}

} // namespace z8::ui
