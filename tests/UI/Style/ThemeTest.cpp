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
  ExpectColor(theme.ScrollBar.TrackColor, Color::PanelSunken);
  ExpectColor(theme.ScrollBar.ThumbColor.Resolve(WidgetVisualState::Normal),
              Color::Divider);
  ExpectColor(theme.Tab.BackgroundColor.Resolve(WidgetVisualState::Hovered),
              Color::ControlHover);
  ExpectColor(theme.Button.ForegroundColor.Resolve(WidgetVisualState::Disabled),
              Color::TextDisabled);
  ExpectColor(theme.Toggle.IndicatorColor.Resolve(WidgetVisualState::Selected),
              Color::Accent);
  ExpectColor(theme.Toggle.IndicatorColor.Resolve(WidgetVisualState::Focused),
              theme.Toggle.IndicatorColor.Normal);
  ExpectColor(theme.Toggle.FocusedBorderColor, Color::Accent);
  ExpectColor(theme.Slider.FillColor, Color::Accent);
  ExpectColor(theme.TextInput.PlaceholderColor, Color::TextMuted);
  ExpectColor(theme.TreeView.RowColor.Resolve(WidgetVisualState::Selected),
              Color::SelectionInactive);
  ExpectColor(theme.Menu.PopupColor, Color::HeaderBackground);
  ExpectColor(theme.ToolBar.Color, Color::HeaderBackground);
  ExpectColor(theme.Dock.PreviewColor, Color::DockPreview);
  ExpectColor(theme.Demo.SelectedRowColor, Color::SelectionInactive);
  EXPECT_FLOAT_EQ(theme.Panel.BorderWidth, 1.0f);
  EXPECT_FLOAT_EQ(theme.ScrollBar.Thickness, 12.0f);
  EXPECT_FLOAT_EQ(theme.Tab.Height, theme.Panel.TitleHeight);
  EXPECT_FLOAT_EQ(theme.Icon.NormalSize, theme.Tab.IconSize);
  EXPECT_GT(theme.Slider.ThumbSize, theme.Slider.TrackThickness);
  EXPECT_GT(theme.TreeView.Indent, 0.0f);
  EXPECT_GT(theme.Menu.PopupWidth, 100.0f);
  EXPECT_EQ(theme.ToolBar.Height, theme.Demo.ToolbarHeight);
  EXPECT_NE(theme.Panel.Color.x, theme.Panel.TitleColor.x);
  EXPECT_LT(theme.Panel.TitleHeight, 36.0f);
  EXPECT_GT(theme.Text.FontSize, 0.0f);
}

TEST(ThemeTest, ResolvesEveryWidgetStateWithoutStringLookup) {
  const auto &colors = Theme::Default().Tab.ForegroundColor;

  ExpectColor(colors.Resolve(WidgetVisualState::Normal), colors.Normal);
  ExpectColor(colors.Resolve(WidgetVisualState::Hovered), colors.Hovered);
  ExpectColor(colors.Resolve(WidgetVisualState::Pressed), colors.Pressed);
  ExpectColor(colors.Resolve(WidgetVisualState::Selected), colors.Selected);
  ExpectColor(colors.Resolve(WidgetVisualState::Disabled), colors.Disabled);
  ExpectColor(colors.Resolve(WidgetVisualState::Focused), colors.Focused);
}

TEST(ThemeTest, KeepsModernAliasOnDefaultEditorTheme) {
  EXPECT_EQ(&Theme::Default(), &Theme::UnrealEditor());
}

} // namespace z8::ui
