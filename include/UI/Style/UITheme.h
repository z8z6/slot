#pragma once

#include <DirectXMath.h>

#include <string>

namespace z8::ui {

/** 单类控件的默认视觉与盒模型，具体实例属性可在此基线之上覆盖。 */
struct UIControlTheme {
  DirectX::XMFLOAT4 Color;
  float Margin = 0.0f;
  float Padding = 0.0f;
  float MinimumWidth = 0.0f;
  float MinimumHeight = 0.0f;
};

/** Panel 除基础盒模型外还统一标题栏、内容留白和交互边界。 */
struct UIPanelTheme : UIControlTheme {
  DirectX::XMFLOAT4 TitleColor;
  DirectX::XMFLOAT4 TitleTextColor;
  DirectX::XMFLOAT4 ScrollBarColor;
  DirectX::XMFLOAT4 ScrollThumbColor;
  float TitleHeight = 36.0f;
  float ContentPadding = 10.0f;
  float ResizeBorder = 6.0f;
  float ScrollBarThickness = 12.0f;
  float MinimumScrollThumbLength = 24.0f;
};

/**
 * 应用级 UI 主题。
 *
 * 当前只提供现代深色主题；集中定义避免各控件构造函数散落魔法颜色和间距。
 */
struct UITheme {
  UIControlTheme Rect;
  UIPanelTheme Panel;

  static const UITheme& Modern();
};

/** 解析 #RRGGBB、#RRGGBBAA 或 r,g,b[,a]，数值通道范围为 0..1。 */
bool ParseUIColor(const std::string& text, DirectX::XMFLOAT4& color);

} // namespace z8::ui
