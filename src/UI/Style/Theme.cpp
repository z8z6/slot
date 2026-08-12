#include "UI/Style/Theme.h"

#include "Util/Color.h"

using namespace DirectX;
using namespace z8::ui;

const Theme& Theme::UnrealEditor() {
  // 颜色取自集中调色板，Theme 只表达“颜色扮演什么角色”和控件盒模型。
  static const Theme theme{
      .Rect = {Color::ControlBackground, Color::Border, 0.0f,
               MarginStyle::Small, PaddingStyle::Small, 30.0f, 24.0f},
      .Text = {Color::Text, Color::TextMuted, Color::TextDisabled, 14.0f,
               20.0f},
      .ScrollBar = {Color::ControlPressed, Color::Divider, 10.0f, 24.0f},
      .Panel = {{Color::PanelBackground, Color::Divider, 1.0f,
                 MarginStyle::Medium, PaddingStyle::Small, 240.0f, 160.0f},
                Color::HeaderBackground, Color::Text, 30.0f,
                PaddingStyle::Medium, 5.0f},
      .Demo = {Color::ControlBackground, Color::ControlBackgroundAlt,
               Color::SelectionInactive, 360.0f, 320.0f, 32.0f,
               MarginStyle::Small, 48.0f, 260.0f, 300.0f, 180.0f}};
  return theme;
}
