#pragma once

#include <DirectXMath.h>


namespace z8::ui {

struct MarginStyle {
  inline static float Large = 10.0f;
  inline static float Medium = 6.0f;
  inline static float Small = 2.0f;
};

struct PaddingStyle {
  inline static float Large = 10.0f;
  inline static float Medium = 6.0f;
  inline static float Small = 2.0f;
};


struct RectStyle {
  DirectX::XMFLOAT4 Color;
  DirectX::XMFLOAT4 BorderColor;
  float BorderWidth = 0.0f;
  float Margin = MarginStyle::Small;
  float Padding = PaddingStyle::Small;
  float MinWidth = 30.0f;
  float MinHeight = 24.0f;
  float CornerRadius = 0.0f;
};


struct TextStyle {
  DirectX::XMFLOAT4 Color;
  DirectX::XMFLOAT4 MutedColor;
  DirectX::XMFLOAT4 DisabledColor;
  float FontSize = 14.0f;
  float LineHeight = 20.0f;
};

struct ScrollBarStyle {
  DirectX::XMFLOAT4 ScrollBarColor;
  DirectX::XMFLOAT4 ScrollThumbColor;
  float ScrollBarThickness = 12.0f;
  float MinimumScrollThumbLength = 24.0f;
};


struct PanelStyle : RectStyle {
  DirectX::XMFLOAT4 TitleColor;
  DirectX::XMFLOAT4 TitleTextColor;

  float TitleHeight = 36.0f;
  float ContentPadding = 10.0f;
  float ResizeBorder = 6.0f;
};

/** Demo 使用主题语义展示控件，避免示例代码重新发明尺寸和颜色。 */
struct DemoStyle {
  DirectX::XMFLOAT4 RowColor;
  DirectX::XMFLOAT4 AlternateRowColor;
  DirectX::XMFLOAT4 SelectedRowColor;
  float PanelWidth = 360.0f;
  float PanelHeight = 320.0f;
  float RowHeight = 32.0f;
  float RowMargin = MarginStyle::Small;
  float ToolbarHeight = 48.0f;
  float LeftPanelWidth = 260.0f;
  float RightPanelWidth = 300.0f;
  float BottomPanelHeight = 180.0f;
};

/**
 * 应用级 UI 主题。
 */
struct Theme {
  RectStyle Rect;
  TextStyle Text;
  ScrollBarStyle ScrollBar;
  PanelStyle Panel;
  DemoStyle Demo;

  static const Theme& UnrealEditor();
  static const Theme& Default() { return UnrealEditor(); }
};

} // namespace z8::ui
