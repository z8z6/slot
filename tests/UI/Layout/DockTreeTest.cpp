#include "UI/Dock/DockTree.h"
#include "UI/Dock/DockWorkspace.h"
#include "UI/Layout/BaseNode.h"
#include "UI/Layout/PanelNode.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(DockTreeTest, BuildsDeterministicNestedSplits) {
  BaseNode viewport;
  BaseNode hierarchy;
  BaseNode console;
  viewport.Key = "Viewport";
  hierarchy.Key = "Hierarchy";
  console.Key = "Console";
  DockTree tree;
  auto *viewportLeaf = tree.AddPanel(&viewport);
  ASSERT_TRUE(tree.Commit({&hierarchy, 0, viewportLeaf->ID, DockSide::Left}));
  auto *currentViewport = tree.FindPanelLeaf(&viewport);
  ASSERT_TRUE(tree.Commit(
      {&console, 0, currentViewport->ID, DockSide::Bottom}));

  tree.Layout({0.0f, 0.0f, 1000.0f, 800.0f});

  EXPECT_FLOAT_EQ(tree.FindPanelLeaf(&hierarchy)->Rect.Width, 500.0f);
  EXPECT_FLOAT_EQ(tree.FindPanelLeaf(&viewport)->Rect.Left, 500.0f);
  EXPECT_FLOAT_EQ(tree.FindPanelLeaf(&viewport)->Rect.Height, 400.0f);
  EXPECT_FLOAT_EQ(tree.FindPanelLeaf(&console)->Rect.Top, 400.0f);
  EXPECT_TRUE(tree.Validate());
}

TEST(DockTreeTest, RejectsImplicitCenterTabCommit) {
  BaseNode viewport;
  BaseNode game;
  DockTree tree;
  auto *leaf = tree.AddPanel(&viewport);

  EXPECT_FALSE(tree.Commit({&game, 0, leaf->ID, DockSide::Center}));

  ASSERT_EQ(tree.Root->Type, DockNodeType::Leaf);
  ASSERT_EQ(tree.Root->Panels.size(), 1U);
  EXPECT_EQ(tree.Root->Panels.front(), &viewport);
  EXPECT_EQ(tree.FindPanelLeaf(&game), nullptr);
  EXPECT_TRUE(tree.Validate());
}

TEST(DockTreeTest, DockingOnlyPanelAgainstItsOwnLeafIsNoOp) {
  BaseNode panel;
  DockTree tree;
  auto *leaf = tree.AddPanel(&panel);
  const auto stableID = leaf->ID;

  ASSERT_TRUE(tree.Commit({&panel, stableID, stableID, DockSide::Right}));

  ASSERT_NE(tree.Root, nullptr);
  EXPECT_EQ(tree.Root->ID, stableID);
  EXPECT_EQ(tree.Root->Type, DockNodeType::Leaf);
  EXPECT_EQ(tree.FindPanelLeaf(&panel), tree.Root.get());
  EXPECT_TRUE(tree.Validate());
}

TEST(DockTreeTest, RemovingLastPanelCollapsesParentSplit) {
  BaseNode left;
  BaseNode right;
  DockTree tree;
  auto *rightLeaf = tree.AddPanel(&right);
  ASSERT_TRUE(tree.Commit({&left, 0, rightLeaf->ID, DockSide::Left}));
  const auto rightID = tree.FindPanelLeaf(&right)->ID;

  ASSERT_TRUE(tree.RemovePanel(&left));

  ASSERT_NE(tree.Root, nullptr);
  EXPECT_EQ(tree.Root->ID, rightID);
  EXPECT_EQ(tree.Root->Parent, nullptr);
  EXPECT_EQ(tree.Root->Type, DockNodeType::Leaf);
  EXPECT_TRUE(tree.Validate());
}

TEST(DockTreeTest, SplitterResizeOnlyChangesRatio) {
  BaseNode left;
  BaseNode right;
  DockTree tree;
  auto *rightLeaf = tree.AddPanel(&right);
  ASSERT_TRUE(tree.Commit({&left, 0, rightLeaf->ID, DockSide::Left}));
  tree.Layout({0.0f, 0.0f, 800.0f, 600.0f});
  const auto oldLeftRect = tree.Root->ChildA->Rect;

  ASSERT_TRUE(tree.ResizeSplitter(tree.Root->ID, 600.0f, 0.0f));

  EXPECT_FLOAT_EQ(tree.Root->SplitRatio, 0.75f);
  // 调整只改结构真值；重新 Layout 前缓存 Rect 保持不动。
  EXPECT_FLOAT_EQ(tree.Root->ChildA->Rect.Width, oldLeftRect.Width);
  tree.Layout({0.0f, 0.0f, 800.0f, 600.0f});
  EXPECT_FLOAT_EQ(tree.Root->ChildA->Rect.Width, 600.0f);
  EXPECT_FLOAT_EQ(tree.Root->ChildB->Rect.Width, 200.0f);
}

TEST(DockTreeTest, SplitterHonorsBothSubtreeMinimums) {
  BaseNode left;
  BaseNode right;
  left.Style.MinWidth = 240.0f;
  right.Style.MinWidth = 180.0f;
  DockTree tree;
  auto *rightLeaf = tree.AddPanel(&right);
  ASSERT_TRUE(tree.Commit({&left, 0, rightLeaf->ID, DockSide::Left}));
  tree.Layout({0.0f, 0.0f, 800.0f, 600.0f});

  ASSERT_TRUE(tree.ResizeSplitter(tree.Root->ID, 10.0f, 0.0f));
  EXPECT_FLOAT_EQ(tree.Root->SplitRatio, 0.3f);
  ASSERT_TRUE(tree.ResizeSplitter(tree.Root->ID, 790.0f, 0.0f));
  EXPECT_FLOAT_EQ(tree.Root->SplitRatio, 0.775f);
}

TEST(DockTreeTest, PreviewUsesThirtyPercentButCommitKeepsEqualSplit) {
  BaseNode first;
  BaseNode second;
  DockTree tree;
  auto *target = tree.AddPanel(&first);
  tree.Layout({0.0f, 0.0f, 1000.0f, 600.0f});

  const auto preview = tree.GetPreviewRect(*target, DockSide::Left);
  EXPECT_FLOAT_EQ(preview.Width, 300.0f);
  ASSERT_TRUE(tree.Commit({&second, 0, target->ID, DockSide::Left}));
  tree.Layout({0.0f, 0.0f, 1000.0f, 600.0f});
  EXPECT_FLOAT_EQ(tree.Root->SplitRatio, 0.5f);
  EXPECT_FLOAT_EQ(tree.Root->ChildA->Rect.Width, 500.0f);
}

TEST(DockWorkspaceTest, DragPreviewDoesNotMutateTreeUntilCommit) {
  PanelNode first;
  PanelNode second;
  DockWorkspace workspace;
  workspace.Reconcile({&first, &second});
  workspace.ApplyLayout(800.0f, 600.0f);
  first.Left = first.Style.Left.value_or(0.0f);
  first.Top = first.Style.Top.value_or(0.0f);
  first.Width = first.Style.Width.value_or(0.0f);
  first.Height = first.Style.Height.value_or(0.0f);
  const auto before = workspace.Tree.Dump();

  workspace.BeginDrag(first, 100.0f, 16.0f);
  workspace.UpdateDrag(700.0f, 300.0f);

  ASSERT_EQ(workspace.Drag.State, PanelDragState::Dragging);
  EXPECT_NE(workspace.Drag.DockTarget, 0U);
  EXPECT_EQ(workspace.Tree.Dump(), before);
  ASSERT_TRUE(workspace.CommitDrag(700.0f, 300.0f));
  EXPECT_NE(workspace.Tree.Dump(), before);
  EXPECT_TRUE(workspace.Validate());
  EXPECT_NE(workspace.DumpDebug().find("DockTree BEFORE"), std::string::npos);
  EXPECT_NE(workspace.DumpDebug().find("Dock Transaction"), std::string::npos);
  EXPECT_NE(workspace.DumpDebug().find("DockTree AFTER"), std::string::npos);
}

TEST(DockWorkspaceTest, DroppingOutsideWorkspaceCreatesFloatingPanel) {
  PanelNode first;
  PanelNode second;
  DockWorkspace workspace;
  workspace.Reconcile({&first, &second});
  workspace.ApplyLayout(800.0f, 600.0f);
  first.Left = first.Style.Left.value_or(0.0f);
  first.Top = first.Style.Top.value_or(0.0f);
  first.Width = first.Style.Width.value_or(0.0f);
  first.Height = first.Style.Height.value_or(0.0f);

  workspace.BeginDrag(first, 100.0f, 16.0f);
  workspace.UpdateDrag(900.0f, 300.0f);
  ASSERT_TRUE(workspace.CommitDrag(900.0f, 300.0f));

  const auto *state = workspace.GetState(first);
  ASSERT_NE(state, nullptr);
  EXPECT_EQ(state->Placement, PanelPlacement::Floating);
  EXPECT_EQ(workspace.Tree.FindPanelLeaf(&first), nullptr);
  EXPECT_FLOAT_EQ(state->FloatingRect.Left, 800.0f);
  EXPECT_TRUE(workspace.Validate());
}

TEST(DockWorkspaceTest, CenterDropCreatesFloatingPanelInsteadOfLeafTab) {
  PanelNode first;
  PanelNode second;
  DockWorkspace workspace;
  workspace.Reconcile({&first, &second});
  workspace.ApplyLayout(800.0f, 600.0f);
  first.Left = first.Style.Left.value_or(0.0f);
  first.Top = first.Style.Top.value_or(0.0f);
  first.Width = first.Style.Width.value_or(0.0f);
  first.Height = first.Style.Height.value_or(0.0f);

  workspace.BeginDrag(first, 100.0f, 16.0f);
  workspace.UpdateDrag(600.0f, 300.0f);
  ASSERT_EQ(workspace.Drag.Side, DockSide::Center);
  ASSERT_TRUE(workspace.CommitDrag(600.0f, 300.0f));

  const auto *state = workspace.GetState(first);
  ASSERT_NE(state, nullptr);
  EXPECT_EQ(state->Placement, PanelPlacement::Floating);
  EXPECT_EQ(workspace.Tree.FindPanelLeaf(&first), nullptr);
  EXPECT_TRUE(workspace.Validate());
}

} // namespace z8::ui
