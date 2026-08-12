#include "UI/Declarative/ImmediateUI.h"
#include "UI/Layout/Layout.h"

#include <gtest/gtest.h>

namespace z8::ui {
namespace {
void DeclareFrame(ImmediateUI& ui, bool includeItem) {
  ui.BeginFrame();
  UIStyle panelStyle;
  panelStyle.Width = 320.0f;
  panelStyle.Height = 180.0f;
  if (ui.BeginPanel("panel", "Inspector", panelStyle)) {
    if (includeItem) ui.Rect("item", UIStyle{.FlexGrow = 1.0f});
    ui.EndPanel();
  }
  ASSERT_TRUE(ui.EndFrame()) << ui.LastError();
}
} // namespace

TEST(ImmediateUITest, ReusesStableControlsAndTracksTopology) {
  Layout layout(nullptr);
  layout.ConsumeDirty();
  ImmediateUI ui(layout);

  DeclareFrame(ui, true);
  auto* firstPanel = layout.Find("panel");
  auto* firstItem = layout.Find("item");
  ASSERT_NE(firstPanel, nullptr);
  ASSERT_NE(firstItem, nullptr);
  EXPECT_TRUE(layout.ConsumeDirty());

  DeclareFrame(ui, true);
  EXPECT_EQ(layout.Find("panel"), firstPanel);
  EXPECT_EQ(layout.Find("item"), firstItem);
  EXPECT_FALSE(layout.ConsumeDirty());

  DeclareFrame(ui, false);
  EXPECT_EQ(layout.Find("panel"), firstPanel);
  EXPECT_EQ(layout.Find("item"), nullptr);
  EXPECT_TRUE(layout.ConsumeDirty());
}

TEST(ImmediateUITest, RejectsDuplicateKeysInEnglish) {
  Layout layout(nullptr);
  ImmediateUI ui(layout);
  ui.BeginFrame();
  ui.Rect("duplicate");
  ui.Rect("duplicate");
  EXPECT_FALSE(ui.EndFrame());
  EXPECT_NE(ui.LastError().find("Duplicate key"), std::string::npos);
}
} // namespace z8::ui
