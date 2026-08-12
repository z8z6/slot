#include "UI/Layout/Layout.h"
#include "UI/Layout/RectNode.h"

#include "yoga/YGNodeLayout.h"

#include <gtest/gtest.h>

namespace z8::ui {
TEST(LayoutTest, BuildsNodeAndRenderableIndexes) {
  Layout layout(nullptr);
  auto child = std::make_unique<RectNode>();
  child->Key = "content";
  layout.Root->AddChild(std::move(child));
  layout.RebuildIndex();

  EXPECT_EQ(layout.Nodes.size(), 2U);
  EXPECT_EQ(layout.Visuals.size(), 1U);
  const auto objects = layout.GetUO();
  ASSERT_EQ(objects.size(), 1U);
  EXPECT_NE(objects.front(), nullptr);
  EXPECT_NE(layout.Find("content"), nullptr);
  EXPECT_TRUE(layout.ConsumeDirty());
  EXPECT_FALSE(layout.ConsumeDirty());
}

TEST(LayoutTest, CalculatesWithoutApplicationOrWindow) {
  Layout layout(nullptr);
  layout.Calculate(800.0f, 600.0f);
  EXPECT_FLOAT_EQ(YGNodeLayoutGetWidth(layout.Root->Node), 800.0f);
  EXPECT_FLOAT_EQ(YGNodeLayoutGetHeight(layout.Root->Node), 600.0f);
}
} // namespace z8::ui
