#include "UI/Declarative/XamlLoader.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/PanelNode.h"

#include <gtest/gtest.h>

namespace z8::ui {
TEST(XamlLoaderTest, BuildsPanelControlTree) {
  constexpr auto source = R"(
    <?xml version="1.0"?>
    <UI Direction="Column">
      <Panel Id="tools" Title="Tools &amp; Scene" Width="300" Height="200"
             TitleHeight="40" Padding="4" DragRegion="Anywhere"
             Scrollable="true" HorizontalScrollEnabled="true"
             HorizontalScrollBar="Auto" VerticalScrollBar="Visible">
        <Rect Id="content" FlexGrow="1" />
      </Panel>
    </UI>)";

  Layout layout(nullptr);
  auto result = XamlLoader().LoadInto(layout, source);
  ASSERT_TRUE(result) << result.Error;
  auto* panel = dynamic_cast<PanelNode*>(layout.Find("tools"));
  ASSERT_NE(panel, nullptr);
  EXPECT_EQ(panel->Title, "Tools & Scene");
  EXPECT_EQ(panel->ContentNode->GetChildCount(), 1U);
  EXPECT_EQ(panel->GetDragProperties().Region, DragRegion::Anywhere);
  EXPECT_TRUE(panel->GetScrollProperties().Horizontal);
  EXPECT_EQ(panel->GetScrollProperties().HorizontalScrollBar,
            ScrollBarVisibility::Auto);
  EXPECT_EQ(panel->GetScrollProperties().VerticalScrollBar,
            ScrollBarVisibility::Visible);
  EXPECT_NE(layout.Find("content"), nullptr);
}

TEST(XamlLoaderTest, ReportsInvalidMarkupInEnglish) {
  XamlLoader loader;
  auto unknown = loader.Load("<Unknown />");
  EXPECT_FALSE(unknown);
  EXPECT_NE(unknown.Error.find("Unknown control type"), std::string::npos);

  auto mismatch = loader.Load("<UI><Rect></UI>");
  EXPECT_FALSE(mismatch);
  EXPECT_GT(mismatch.ErrorOffset, 0U);
  EXPECT_NE(mismatch.Error.find("Mismatched closing tag"), std::string::npos);

  auto duplicate = loader.Load("<UI><Rect Id=\"same\"/><Rect Id=\"same\"/></UI>");
  EXPECT_FALSE(duplicate);
  EXPECT_NE(duplicate.Error.find("Duplicate control key"), std::string::npos);
}
} // namespace z8::ui
