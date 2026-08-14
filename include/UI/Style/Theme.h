#pragma once

#include <DirectXMath.h>

namespace z8::ui {

/** 编辑器控件共享的视觉状态；顺序不表示优先级，由控件按交互语义选择。 */
enum class WidgetVisualState {
  Normal,
  Hovered,
  Pressed,
  Selected,
  Disabled,
  Focused
};

/**
 * 一组状态颜色映射。
 *
 * 控件只负责决定当前语义状态，颜色选择集中在 Theme，避免 Tab、按钮和滚动条
 * 各自复制 Hover/Pressed 判断与色值。
 */
struct StateColorStyle {
  DirectX::XMFLOAT4 Normal;
  DirectX::XMFLOAT4 Hovered;
  DirectX::XMFLOAT4 Pressed;
  DirectX::XMFLOAT4 Selected;
  DirectX::XMFLOAT4 Disabled;
  DirectX::XMFLOAT4 Focused;

  const DirectX::XMFLOAT4 &Resolve(WidgetVisualState state) const;
};

/** 紧凑桌面编辑器使用的统一间距刻度，避免控件内部出现随机留白。 */
struct SpacingStyle {
  inline static constexpr float ExtraSmall = 2.0f;
  inline static constexpr float Small = 4.0f;
  inline static constexpr float Medium = 6.0f;
  inline static constexpr float Large = 8.0f;
};

/** 普通矩形的盒模型默认值；具体复合控件在此基础上使用自己的语义样式。 */
struct RectStyle {
  DirectX::XMFLOAT4 Color;
  DirectX::XMFLOAT4 BorderColor;
  float BorderWidth = 0.0f;
  float Margin = SpacingStyle::ExtraSmall;
  float Padding = SpacingStyle::ExtraSmall;
  float MinWidth = 30.0f;
  float MinHeight = 24.0f;
  float CornerRadius = 0.0f;
};

/** 编辑器文字层级；目前由 DirectWrite 使用同一字体族，仅区分尺寸和语义色。 */
struct TextStyle {
  DirectX::XMFLOAT4 Color;
  DirectX::XMFLOAT4 MutedColor;
  DirectX::XMFLOAT4 DisabledColor;
  float FontSize = 14.0f;
  float SmallFontSize = 12.0f;
  float HeadingFontSize = 15.0f;
  float LineHeight = 20.0f;
};

/** 图标尺寸和着色层级；图标节点只保存语义，不在业务控件内指定像素尺寸。 */
struct IconStyle {
  StateColorStyle Color;
  float SmallSize = 12.0f;
  float NormalSize = 16.0f;
  float LargeSize = 20.0f;
};

/** 普通按钮与无边框图标按钮共享的紧凑状态样式。 */
struct ButtonStyle {
  StateColorStyle BackgroundColor;
  StateColorStyle ForegroundColor;
  DirectX::XMFLOAT4 BorderColor;
  float BorderWidth = 0.0f;
  float ControlHeight = 24.0f;
  float IconButtonSize = 28.0f;
  float CornerRadius = 2.0f;
  float ContentPadding = SpacingStyle::Small;
};

/** Dock 拖放反馈独立于 Panel 本体，透明度和边框宽度仍由主题统一控制。 */
struct DockStyle {
  DirectX::XMFLOAT4 PreviewColor;
  DirectX::XMFLOAT4 PreviewBorderColor;
  float PreviewBorderWidth = 2.0f;
};

/** PanelGroup 页签专用样式；布局尺寸与状态颜色必须来自同一主题。 */
struct TabStyle {
  StateColorStyle BackgroundColor;
  StateColorStyle ForegroundColor;
  StateColorStyle IconColor;
  DirectX::XMFLOAT4 AccentColor;
  DirectX::XMFLOAT4 SeparatorColor;
  float Height = 28.0f;
  float MinWidth = 72.0f;
  float MaxWidth = 240.0f;
  float IconSize = 16.0f;
  float IconMargin = SpacingStyle::Medium;
  float FixedContentWidth = 48.0f;
  float AccentHeight = 2.0f;
  float CornerRadius = 0.0f;
};

/** 低对比滚动条样式；轨道命中宽度与内缩后的滑块视觉宽度彼此独立。 */
struct ScrollBarStyle {
  DirectX::XMFLOAT4 TrackColor;
  StateColorStyle ThumbColor;
  float Thickness = 12.0f;
  float ThumbInset = 3.0f;
  float MinimumThumbLength = 24.0f;
  float CornerRadius = 2.0f;
};

/** Panel 与 PanelGroup 共用的表面、标题栏和内容分隔语义。 */
struct PanelStyle : RectStyle {
  DirectX::XMFLOAT4 TitleColor;
  DirectX::XMFLOAT4 TitleActiveColor;
  DirectX::XMFLOAT4 TitleTextColor;
  DirectX::XMFLOAT4 ContentSeparatorColor;
  float TitleHeight = 28.0f;
  float ContentPadding = SpacingStyle::Medium;
  float ContentSeparatorWidth = 1.0f;
  float ResizeBorder = 6.0f;
};

/** Demo 使用主题语义展示控件，避免示例代码重新发明尺寸和颜色。 */
struct DemoStyle {
  DirectX::XMFLOAT4 RowColor;
  DirectX::XMFLOAT4 AlternateRowColor;
  DirectX::XMFLOAT4 SelectedRowColor;
  float PanelWidth = 360.0f;
  float PanelHeight = 320.0f;
  float RowHeight = 28.0f;
  float RowMargin = SpacingStyle::ExtraSmall;
  float ToolbarHeight = 36.0f;
  float LeftPanelWidth = 260.0f;
  float RightPanelWidth = 300.0f;
  float BottomPanelHeight = 180.0f;
};

/**
 * 应用级 UI 主题。
 *
 * Theme 是稳定的只读值对象；控件构造时复制少量 token 到布局与渲染数据，
 * 每帧状态切换只解析颜色，不进行字符串查找、分配或资源创建。
 */
struct Theme {
  RectStyle Rect;
  TextStyle Text;
  IconStyle Icon;
  ButtonStyle Button;
  DockStyle Dock;
  TabStyle Tab;
  ScrollBarStyle ScrollBar;
  PanelStyle Panel;
  DemoStyle Demo;

  static const Theme &Default() { return UnrealEditor(); }
  static const Theme &UnrealEditor();
};

} // namespace z8::ui
