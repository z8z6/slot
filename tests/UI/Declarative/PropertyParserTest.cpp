#include "UI/Property/PropertyParser.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(PropertyParserTest, AcceptsOnlyDocumentedBooleanLiterals) {
  bool value = false;
  EXPECT_TRUE(ParseBoolean("true", value));
  EXPECT_TRUE(value);
  EXPECT_TRUE(ParseBoolean("0", value));
  EXPECT_FALSE(value);

  // 声明语法必须跨控件保持一致，不能把拼写近似值静默解释为 false。
  EXPECT_FALSE(ParseBoolean("TRUE", value));
  EXPECT_FALSE(ParseBoolean("yes", value));
  EXPECT_FALSE(ParseBoolean("2", value));
}

TEST(PropertyParserTest, AcceptsOnlyCompleteFiniteFloats) {
  float value = 0.0f;
  EXPECT_TRUE(ParseFiniteFloat("-12.5", value));
  EXPECT_FLOAT_EQ(value, -12.5f);
  EXPECT_TRUE(ParseFiniteFloat("1e2", value));
  EXPECT_FLOAT_EQ(value, 100.0f);

  // 单位后缀、非有限值与空值都不能进入布局求解器或 GPU 常量。
  EXPECT_FALSE(ParseFiniteFloat("12px", value));
  EXPECT_FLOAT_EQ(value, 100.0f);
  EXPECT_FALSE(ParseFiniteFloat("nan", value));
  EXPECT_FALSE(ParseFiniteFloat("inf", value));
  EXPECT_FALSE(ParseFiniteFloat("", value));
}

} // namespace z8::ui
