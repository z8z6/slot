#include "UI/Layout/Layout.h"
#include "UI/Layout/RectNode.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(LayoutTest, BuildsNodeAndRenderableIndexes) {
  Layout layout;
  auto child = std::make_unique<RectNode>();
  child->Key = "content";
  layout.Root->AddChild(std::move(child));
  layout.RebuildIndex();

  EXPECT_EQ(layout.Nodes.size(), 2U);
  EXPECT_EQ(layout.Visuals.size(), 1U);
  EXPECT_NE(layout.Find("content"), nullptr);
  EXPECT_TRUE(layout.ConsumeDirty());
}

} // namespace z8::ui
