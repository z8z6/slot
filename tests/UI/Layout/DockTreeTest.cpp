#include "UI/Dock/DockTree.h"
#include "UI/Layout/BaseNode.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(DockTreeTest, BuildsAndLaysOutSplitTree) {
  BaseNode viewport;
  BaseNode hierarchy;
  DockTree tree;
  auto* viewportLeaf = tree.AddPanel(&viewport);
  ASSERT_TRUE(
      tree.Commit({&hierarchy, 0, viewportLeaf->ID, DockSide::Left}));

  tree.Layout({0.0f, 0.0f, 1000.0f, 800.0f});

  EXPECT_FLOAT_EQ(tree.FindPanelLeaf(&hierarchy)->Rect.Width, 500.0f);
  EXPECT_FLOAT_EQ(tree.FindPanelLeaf(&viewport)->Rect.Left, 500.0f);
  EXPECT_TRUE(tree.Validate());
}

} // namespace z8::ui
