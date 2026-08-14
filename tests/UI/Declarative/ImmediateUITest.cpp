#include "UI/Declarative/ImmediateUI.h"
#include "UI/Layout/Layout.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(ImmediateUITest, ReusesStableControlsAcrossFrames) {
  Layout layout;
  ImmediateUI ui(layout);

  ui.BeginFrame();
  ui.Rect("item");
  ASSERT_TRUE(ui.EndFrame()) << ui.LastError();
  auto* first = layout.Find("item");
  ASSERT_NE(first, nullptr);

  ui.BeginFrame();
  ui.Rect("item");
  ASSERT_TRUE(ui.EndFrame()) << ui.LastError();
  EXPECT_EQ(layout.Find("item"), first);
}

} // namespace z8::ui
