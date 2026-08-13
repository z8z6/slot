#include "UI/Declarative/XamlLoader.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/PanelNode.h"
#include "UI/Layout/PanelGroupNode.h"
#include "UI/Layout/SceneNode.h"
#include "UI/Layout/TerminalNode.h"

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

  Layout layout;
  auto result = XamlLoader().LoadInto(layout, source);
  ASSERT_TRUE(result) << result.Error;
  auto *panel = dynamic_cast<PanelNode *>(layout.Find("tools"));
  ASSERT_NE(panel, nullptr);
  EXPECT_EQ(panel->TitleNode->Text, "Tools & Scene");
  EXPECT_EQ(panel->ScrollAreaNode->ContentNode->Children.size(), 1U);
  const auto *drag = panel->GetBehavior<DragBehavior>();
  const auto *scroll = panel->ScrollAreaNode->GetScrollBehavior();
  ASSERT_NE(drag, nullptr);
  ASSERT_NE(scroll, nullptr);
  EXPECT_EQ(drag->Properties.Region, DragRegion::Anywhere);
  EXPECT_TRUE(scroll->Properties.Horizontal);
  EXPECT_EQ(scroll->Properties.HorizontalScrollBar,
            ScrollBarVisibility::Auto);
  EXPECT_EQ(scroll->Properties.VerticalScrollBar,
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

  auto duplicate =
      loader.Load("<UI><Rect Id=\"same\"/><Rect Id=\"same\"/></UI>");
  EXPECT_FALSE(duplicate);
  EXPECT_NE(duplicate.Error.find("Duplicate control key"), std::string::npos);
}

TEST(XamlLoaderTest, CreatesSceneViewportNode) {
  Layout layout;
  const auto result = XamlLoader().LoadInto(
      layout, "<UI><Scene Id=\"viewport\" /></UI>");

  ASSERT_TRUE(result) << result.Error;
  ASSERT_NE(layout.GetSceneNode(), nullptr);
  EXPECT_EQ(static_cast<BaseNode *>(layout.GetSceneNode()),
            layout.Find("viewport"));
}

TEST(XamlLoaderTest, CreatesTerminalNode) {
  Layout layout;
  XamlLoader loader;
  const auto result = loader.LoadInto(
      layout, "<UI><Terminal Id=\"output\" Title=\"Messages\" /></UI>");
  ASSERT_TRUE(result) << result.Error;

  auto *terminal = dynamic_cast<TerminalNode *>(layout.Find("output"));
  ASSERT_NE(terminal, nullptr);
  EXPECT_EQ(terminal->TitleNode->Text, "Messages");
}

TEST(XamlLoaderTest, CreatesPanelGroupWithSwitchablePanels) {
  Layout layout;
  const auto result = XamlLoader().LoadInto(
      layout,
      "<UI><PanelGroup Id=\"editors\"><Panel Title=\"Scene\"/>"
      "<Panel Title=\"Game\"/></PanelGroup></UI>");
  ASSERT_TRUE(result) << result.Error;

  auto *group = dynamic_cast<PanelGroupNode *>(layout.Find("editors"));
  ASSERT_NE(group, nullptr);
  ASSERT_EQ(group->Panels.size(), 2U);
  EXPECT_EQ(group->Tabs[0]->LabelNode->Text, "Scene");
  EXPECT_EQ(group->Tabs[1]->LabelNode->Text, "Game");
  EXPECT_TRUE(group->Panels[0]->Visible);
  EXPECT_FALSE(group->Panels[1]->Visible);
}
} // namespace z8::ui
