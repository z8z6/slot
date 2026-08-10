#include "UI/Layout/PanelNode.h"

#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"

#include <gtest/gtest.h>

namespace z8::ui {
TEST(PanelNodeTest, KeepsTitleAndContentAsInternalChildren) {
  PanelNode panel;
  ASSERT_NE(panel.TitleNode, nullptr);
  ASSERT_NE(panel.ContentNode, nullptr);
  EXPECT_EQ(panel.GetChildCount(), 2U);
  EXPECT_EQ(panel.ContentHost(), panel.ContentNode);

  auto content = std::make_unique<RectNode>();
  auto* contentObserver = content.get();
  panel.ContentHost()->AddChild(std::move(content));
  EXPECT_EQ(panel.ContentNode->GetChild(0), contentObserver);
  EXPECT_EQ(panel.TitleNode->GetChildCount(), 0U);
}

TEST(PanelNodeTest, AppliesTitleAndTitleHeight) {
  PanelNode panel;
  EXPECT_TRUE(panel.SetProperty("Title", "Inspector"));
  EXPECT_TRUE(panel.SetProperty("TitleHeight", "40"));
  EXPECT_EQ(panel.Title, "Inspector");

  YGNodeStyleSetWidth(panel.GetYogaNode(), 300.0f);
  YGNodeStyleSetHeight(panel.GetYogaNode(), 200.0f);
  YGNodeCalculateLayout(panel.GetYogaNode(), 300.0f, 200.0f, YGDirectionLTR);
  EXPECT_FLOAT_EQ(YGNodeLayoutGetHeight(panel.TitleNode->GetYogaNode()), 40.0f);
}
} // namespace z8::ui
