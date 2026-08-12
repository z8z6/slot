//
// Created by zhou_zhengming on 2026/5/16.
//

#pragma once

#include <DirectXMath.h>
#include <string>

namespace z8 {

/** 在编译期把 8 位通道转换为框架使用的浮点颜色。 */
constexpr DirectX::XMFLOAT4 ColorFromBytes(unsigned red, unsigned green,
                                           unsigned blue,
                                           unsigned alpha = 255) {
  return {red / 255.0f, green / 255.0f, blue / 255.0f,
          alpha / 255.0f};
}

class Color {
public:
  // UE 编辑器式深色层级：从窗口背景到抬升控件逐级变亮，避免纯黑造成高反差。
  inline static constexpr auto EditorBackground = ColorFromBytes(17, 17, 17);
  inline static constexpr auto PanelBackground = ColorFromBytes(26, 26, 26);
  inline static constexpr auto HeaderBackground = ColorFromBytes(36, 36, 36);
  inline static constexpr auto ControlBackground = ColorFromBytes(42, 42, 42);
  inline static constexpr auto ControlBackgroundAlt = ColorFromBytes(34, 34, 34);
  inline static constexpr auto ControlHover = ColorFromBytes(52, 52, 52);
  inline static constexpr auto ControlPressed = ColorFromBytes(24, 24, 24);
  inline static constexpr auto Border = ColorFromBytes(8, 8, 8);
  inline static constexpr auto Divider = ColorFromBytes(55, 55, 55);

  // UE 编辑器常用的蓝色选择语义；弱强调用于行底色，强强调用于焦点和选中。
  inline static constexpr auto Accent = ColorFromBytes(0, 112, 224);
  inline static constexpr auto AccentHover = ColorFromBytes(26, 140, 255);
  inline static constexpr auto Selection = ColorFromBytes(0, 96, 192);
  inline static constexpr auto SelectionInactive = ColorFromBytes(52, 73, 94);

  inline static constexpr auto Text = ColorFromBytes(220, 220, 220);
  inline static constexpr auto TextMuted = ColorFromBytes(160, 160, 160);
  inline static constexpr auto TextDisabled = ColorFromBytes(96, 96, 96);
  inline static constexpr auto Warning = ColorFromBytes(230, 158, 42);
  inline static constexpr auto Error = ColorFromBytes(220, 64, 64);
  inline static constexpr auto Success = ColorFromBytes(74, 166, 91);

  // 保留渲染清屏所需 XMVECTORF32 ABI；名称使用语义而非亮度序号。
  inline static const DirectX::XMVECTORF32 Clear = {
      {{EditorBackground.x, EditorBackground.y, EditorBackground.z,
        EditorBackground.w}}};
};

/**
 * 解析 #RRGGBB、#RRGGBBAA 或 r,g,b[,a]，数值通道范围为 0..1
 */
bool ParseUIColor(const std::string& text, DirectX::XMFLOAT4& color);

} // namespace z8
