#include "UI/Declarative/XamlLoader.h"
#include "UI/Layout/Layout.h"
#include "UI/Layout/PanelNode.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(XamlLoaderTest, BuildsNamedControlTree) {
  constexpr auto source = R"(
    <UI Direction="Column">
      <Panel Id="tools" Title="Tools" Width="300" Height="200">
        <Rect Id="content" FlexGrow="1" />
      </Panel>
    </UI>)";

  Layout layout;
  auto result = XamlLoader().LoadInto(layout, source);

  ASSERT_TRUE(result) << result.Error;
  auto* panel = dynamic_cast<PanelNode*>(layout.Find("tools"));
  ASSERT_NE(panel, nullptr);
  EXPECT_NE(layout.Find("content"), nullptr);
  EXPECT_EQ(panel->TitleNode->Text, "Tools");
}

} // namespace z8::ui
