#include "UI/Style/Theme.h"
#include "Util/Color.h"

#include <gtest/gtest.h>

namespace z8::ui {

TEST(ThemeTest, MapsCoreControlRoles) {
  const auto& theme = Theme::UnrealEditor();

  EXPECT_FLOAT_EQ(theme.Panel.Color.x, Color::PanelBackground.x);
  EXPECT_FLOAT_EQ(theme.Panel.TitleColor.x, Color::HeaderBackground.x);
  EXPECT_FLOAT_EQ(theme.Button.ForegroundColor
                      .Resolve(WidgetVisualState::Disabled)
                      .w,
                  Color::TextDisabled.w);
  EXPECT_FLOAT_EQ(theme.Panel.BorderWidth, 1.0f);
}

} // namespace z8::ui
