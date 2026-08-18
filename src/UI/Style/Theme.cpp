#include "UI/Style/Theme.h"

#include "Util/Color.h"

using namespace z8::ui;

const DirectX::XMFLOAT4 &
StateColorStyle::Resolve(WidgetVisualState state) const {
  switch (state) {
  case WidgetVisualState::Hovered:
    return Hovered;
  case WidgetVisualState::Pressed:
    return Pressed;
  case WidgetVisualState::Selected:
    return Selected;
  case WidgetVisualState::Disabled:
    return Disabled;
  case WidgetVisualState::Focused:
    return Focused;
  case WidgetVisualState::Normal:
  default:
    return Normal;
  }
}

const Theme &Theme::UnrealEditor() {
  // Theme 在一个位置建立 Surface、交互和文字层级；控件仅消费语义字段，
  // 后续视觉调色不会再触碰 PanelGroup 或 ScrollBar 的控制流。
  // C++17 没有指定初始化，下列顺序因此必须与 Theme 成员声明保持一致。
  static const Theme theme{
      {Color::ControlBackground, Color::Border, 0.0f,
               SpacingStyle::ExtraSmall, SpacingStyle::ExtraSmall, 30.0f, 24.0f,
               2.0f},
      // 微软雅黑覆盖中英文常用字形，避免 Segoe UI 绘制中文时依赖系统回退，
      // 从而让同一行中的中英文保持一致的 hinting、基线和视觉粗细。
      {Color::Text, Color::TextMuted, Color::TextDisabled,
               L"Microsoft YaHei", 14.0f, 12.0f, 15.0f, 20.0f},
      {{Color::TextMuted, Color::Text, Color::Text, Color::Text,
                Color::TextDisabled, Color::Accent},
               12.0f,
               16.0f,
               20.0f},
      {{Color::Transparent, Color::ControlHover,
                  Color::ControlPressed, Color::SelectionInactive,
                  Color::Transparent, Color::ControlHover},
                 {Color::TextMuted, Color::Text, Color::Text, Color::Text,
                  Color::TextDisabled, Color::Text},
                 Color::Border,
                 0.0f,
                 24.0f,
                 28.0f,
                 2.0f,
                 SpacingStyle::Small},
      {{Color::PanelSunken, Color::ControlHover,
                  Color::ControlPressed, Color::Accent,
                  Color::ControlBackground, Color::PanelSunken},
                 {Color::TextMuted, Color::Text, Color::Text, Color::Text,
                  Color::TextDisabled, Color::Text},
                 Color::BorderSubtle,
                 Color::Accent,
                 1.0f,
                 24.0f,
                 14.0f,
                 2.0f,
                 SpacingStyle::Medium},
      {Color::PanelSunken,
                 Color::Accent,
                 {Color::TextMuted, Color::Text, Color::AccentHover,
                  Color::Accent, Color::TextDisabled, Color::Accent},
                 24.0f,
                 96.0f,
                 4.0f,
                 12.0f,
                 2.0f},
      {{Color::ControlBackground, Color::ControlBackground,
                     Color::ControlPressed, Color::ControlBackground,
                     Color::PanelSunken, Color::ControlBackground},
                    {Color::Text, Color::Text, Color::Text, Color::Text,
                     Color::TextDisabled, Color::Text},
                    Color::TextMuted,
                    Color::BorderSubtle,
                    Color::Accent,
                    Color::Text,
                    24.0f,
                    120.0f,
                    SpacingStyle::Medium,
                    1.0f,
                    1.0f,
                    2.0f},
      {{Color::Transparent, Color::ControlHover,
                    Color::ControlPressed, Color::SelectionInactive,
                    Color::Transparent, Color::ControlHover},
                   {Color::TextMuted, Color::Text, Color::Text, Color::Text,
                    Color::TextDisabled, Color::Text},
                   {Color::TextMuted, Color::Text, Color::Text, Color::Text,
                    Color::TextDisabled, Color::Accent},
                   24.0f,
                   12.0f,
                   12.0f,
                   SpacingStyle::Small},
      {{Color::Transparent, Color::ControlHover,
                Color::ControlPressed, Color::SelectionInactive,
                Color::Transparent, Color::ControlHover},
               {Color::TextMuted, Color::Text, Color::Text, Color::Text,
                Color::TextDisabled, Color::Text},
               Color::HeaderBackground,
               Color::Border,
               28.0f,
               24.0f,
               184.0f,
               SpacingStyle::Medium,
               SpacingStyle::ExtraSmall,
               12.0f,
               1.0f,
               2.0f},
      {Color::HeaderBackground, Color::Divider, 36.0f,
                  SpacingStyle::Small, 1.0f},
      {Color::DockPreview, Color::DockPreviewBorder, 2.0f},
      {{Color::HeaderBackground, Color::ControlHover,
               Color::ControlPressed, Color::PanelBackground,
               Color::HeaderBackground, Color::ControlHover},
              {Color::TextMuted, Color::Text, Color::Text, Color::Text,
               Color::TextDisabled, Color::Text},
              {Color::TextMuted, Color::Text, Color::Text, Color::Text,
               Color::TextDisabled, Color::Text},
              Color::Accent,
              Color::Divider,
              28.0f,
              72.0f,
              240.0f,
              16.0f,
              SpacingStyle::Medium,
              48.0f,
              2.0f,
              0.0f},
      // 12px 命中轨道两侧各内缩 3px，得到克制的 6px Thumb；悬停和拖动
      // 仅改变 Thumb 对比度，避免滚动条成为内容区视觉焦点。
      {Color::PanelSunken,
                    {Color::Divider, Color::TextDisabled, Color::Accent,
                     Color::Divider, Color::TextDisabled, Color::Accent},
                    12.0f,
                    3.0f,
                    24.0f,
                    2.0f},
      {{Color::PanelBackground, Color::BorderSubtle, 1.0f, 0.0f, 0.0f,
                 240.0f, 160.0f, 0.0f},
                Color::HeaderBackground,
                Color::HeaderActive,
                Color::Text,
                Color::Divider,
                28.0f,
                SpacingStyle::Medium,
                1.0f,
                5.0f},
      {Color::ControlBackground, Color::ControlBackgroundAlt,
               Color::SelectionInactive, 360.0f, 320.0f, 28.0f,
               SpacingStyle::ExtraSmall, 36.0f, 260.0f, 300.0f, 180.0f}};
  return theme;
}
