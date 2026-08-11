#include "UI/Layout/RectNode.h"
#include "Object/UIObject/RectUIObject.h"

#include <gtest/gtest.h>

namespace z8::ui {
TEST(RectNodeTest, CreatesRectVisual) {
  RectNode node;
  EXPECT_STREQ(node.TypeName(), "Rect");
  EXPECT_NE(dynamic_cast<RectUIObject*>(node.GetUO()), nullptr);
  EXPECT_EQ(YGNodeGetContext(node.GetYogaNode()), &node);
}
} // namespace z8::ui
