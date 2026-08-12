#include "UI/Declarative/ControlFactory.h"
#include "UI/Layout/BehaviorNode.h"
#include "UI/Layout/DrawNode.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/TextNode.h"

#include <gtest/gtest.h>
#include <type_traits>

namespace z8::ui {

TEST(TextNodeTest, IsPureLayoutTextAndRegistersWithLayout) {
  static_assert(!std::is_base_of_v<BehaviorNode, TextNode>);
  static_assert(!std::is_base_of_v<DrawNode, TextNode>);

  Layout layout;
  auto text = std::make_unique<TextNode>();
  auto *observer = text.get();
  ASSERT_TRUE(text->SetProperty("Text", "Hello, Slot"));
  ASSERT_TRUE(text->SetProperty("FontSize", "18"));
  ASSERT_TRUE(text->SetProperty("TextAlignment", "Center"));
  layout.Root->AddChild(std::move(text));
  layout.RebuildIndex();

  ASSERT_EQ(layout.Texts.size(), 1U);
  EXPECT_EQ(layout.Texts.front(), observer);
  EXPECT_TRUE(layout.Visuals.empty());
  EXPECT_EQ(observer->Text, "Hello, Slot");
  EXPECT_FLOAT_EQ(observer->FontSize, 18.0f);
  EXPECT_EQ(observer->Alignment, TextAlignment::Center);
}

TEST(TextNodeTest, IsAvailableToDeclarativeConstruction) {
  auto node = ControlFactory::Instance().Create("Text");
  EXPECT_NE(dynamic_cast<TextNode *>(node.get()), nullptr);
}

} // namespace z8::ui
