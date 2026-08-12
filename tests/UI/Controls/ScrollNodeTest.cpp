#include "UI/Layout/ScrollNode.h"
#include "UI/Layout/Layout.h"
#include "yoga/YGNodeStyle.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(ScrollNodeTest, OwnsReusableScrollComposition) {
  ScrollNode scroll;
  ASSERT_NE(scroll.ViewportNode, nullptr);
  ASSERT_NE(scroll.ContentNode, nullptr);
  ASSERT_NE(scroll.VerticalScrollBarNode, nullptr);
  EXPECT_EQ(scroll.ContentHost(), scroll.ContentNode);
  EXPECT_EQ(scroll.ContentNode->Parent, scroll.ViewportNode);
  EXPECT_NE(scroll.GetScrollBehavior(), nullptr);
}

TEST(ScrollNodeTest, ReceivesInputAcrossAnEmptyViewport) {
  Layout layout;
  auto scroll = std::make_unique<ScrollNode>();
  auto *observer = scroll.get();
  YGNodeStyleSetWidth(scroll->Node, 200.0f);
  YGNodeStyleSetHeight(scroll->Node, 120.0f);
  layout.Root->AddChild(std::move(scroll));
  layout.RebuildIndex();
  layout.Calculate(400.0f, 300.0f);

  MouseMovArgs pointer;
  pointer.X = 50;
  pointer.Y = 50;
  EXPECT_TRUE(observer->Contains(static_cast<float>(pointer.X),
                                 static_cast<float>(pointer.Y)));
  EXPECT_EQ(layout.OnMouseMove(pointer), EventReply::Handled);
}

} // namespace z8::ui
