#include "UI/Layout/RectNode.h"
#include "Object/UIObject/RectUIObject.h"
#include "UI/Property/IProperty.h"
#include "UI/Style/Theme.h"

#include <gtest/gtest.h>

namespace z8::ui {
TEST(RectNodeTest, ImplementsCommonPropertyInterface) {
  RectNode rect;

  // 基础控件也能被声明前端通过统一接口设置，不要求知道其具体 C++ 类型。
  auto *property = dynamic_cast<IProperty *>(&rect);
  ASSERT_NE(property, nullptr);
  EXPECT_TRUE(property->SetProperty("Width", "120"));
}

TEST(RectNodeTest, CreatesRectVisual) {
  RectNode node;
  EXPECT_STREQ(node.TypeName(), "Rect");
  EXPECT_NE(dynamic_cast<RectUIObject *>(node.UO.get()), nullptr);
  const auto &theme = Theme::Default().Rect;
  EXPECT_FLOAT_EQ(node.Style.Margin, theme.Margin);
  EXPECT_FLOAT_EQ(node.UO->GetColor().x, theme.Color.x);
  EXPECT_FLOAT_EQ(node.UO->GetColor().y, theme.Color.y);
}

TEST(RectNodeTest, OverridesThemeColorWithUnifiedProperty) {
  RectNode node;
  ASSERT_TRUE(node.SetProperty("Color", "#336699CC"));
  EXPECT_NEAR(node.UO->GetColor().x, 0.2f, 0.001f);
  EXPECT_NEAR(node.UO->GetColor().y, 0.4f, 0.001f);
  EXPECT_NEAR(node.UO->GetColor().z, 0.6f, 0.001f);
  EXPECT_NEAR(node.UO->GetColor().w, 0.8f, 0.001f);
  EXPECT_FALSE(node.SetProperty("Color", "not-a-color"));
}

TEST(RectNodeTest, SupportsPixelBorderProperties) {
  RectNode node;
  ASSERT_TRUE(node.SetProperty("BorderColor", "#80A0C0FF"));
  ASSERT_TRUE(node.SetProperty("Border", "2"));

  EXPECT_NEAR(node.UO->GetBorderColor().x, 128.0f / 255.0f, 0.001f);
  EXPECT_FLOAT_EQ(node.UO->GetBorderWidth(), 2.0f);
  EXPECT_FALSE(node.SetProperty("BorderWidth", "-1"));
}
} // namespace z8::ui
