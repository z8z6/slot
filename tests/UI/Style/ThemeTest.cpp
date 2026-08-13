#include "UI/Style/Theme.h"
#include "Util/Color.h"

#include <gtest/gtest.h>

namespace z8::ui {
namespace {

void ExpectColor(const DirectX::XMFLOAT4 &actual,
                 const DirectX::XMFLOAT4 &expected) {
  EXPECT_FLOAT_EQ(actual.x, expected.x);
  EXPECT_FLOAT_EQ(actual.y, expected.y);
  EXPECT_FLOAT_EQ(actual.z, expected.z);
  EXPECT_FLOAT_EQ(actual.w, expected.w);
}

} // namespace

TEST(ThemeTest, MapsUnrealEditorPaletteToControlRoles) {
  const auto &theme = Theme::UnrealEditor();
  ExpectColor(theme.Panel.Color, Color::PanelBackground);
  ExpectColor(theme.Panel.TitleColor, Color::HeaderBackground);
  ExpectColor(theme.Panel.TitleTextColor, Color::Text);
  ExpectColor(theme.ScrollBar.ScrollBarColor, Color::ControlPressed);
  ExpectColor(theme.ScrollBar.ScrollThumbColor, Color::Divider);
  ExpectColor(theme.Demo.SelectedRowColor, Color::SelectionInactive);
  EXPECT_FLOAT_EQ(theme.Panel.BorderWidth, 2.0f);
  EXPECT_NE(theme.Panel.Color.x, theme.Panel.TitleColor.x);
  EXPECT_LT(theme.Panel.TitleHeight, 36.0f);
  EXPECT_GT(theme.Text.FontSize, 0.0f);
}

TEST(ThemeTest, KeepsModernAliasOnDefaultEditorTheme) {
  EXPECT_EQ(&Theme::Default(), &Theme::UnrealEditor());
}

} // namespace z8::ui
