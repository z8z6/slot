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
  return {red / 255.0f, green / 255.0f, blue / 255.0f, alpha / 255.0f};
}

class Color {
public:
  // 中性暗灰表面通过小幅明度差建立 Workspace、Panel、Header 和 Control
  // 层级；降低蓝色底色占比后，Accent 只在选中和焦点语义中出现。
  inline static constexpr auto EditorBackground = ColorFromBytes(21, 22, 24);
  inline static constexpr auto PanelBackground = ColorFromBytes(29, 30, 33);
  inline static constexpr auto PanelSunken = ColorFromBytes(23, 24, 27);
  inline static constexpr auto HeaderBackground = ColorFromBytes(35, 36, 39);
  inline static constexpr auto HeaderActive = ColorFromBytes(40, 41, 45);
  inline static constexpr auto ControlBackground = ColorFromBytes(39, 40, 44);
  inline static constexpr auto ControlBackgroundAlt =
      ColorFromBytes(32, 33, 36);
  inline static constexpr auto ControlHover = ColorFromBytes(49, 50, 55);
  inline static constexpr auto ControlPressed = ColorFromBytes(27, 28, 31);
  inline static constexpr auto Border = ColorFromBytes(57, 58, 63);
  inline static constexpr auto BorderSubtle = ColorFromBytes(43, 44, 48);
  inline static constexpr auto Divider = ColorFromBytes(51, 52, 57);
  inline static constexpr auto Transparent = ColorFromBytes(0, 0, 0, 0);

  // UE 编辑器常用的蓝色选择语义；弱强调用于行底色，强强调用于焦点和选中。
  inline static constexpr auto Accent = ColorFromBytes(53, 120, 184);
  inline static constexpr auto AccentHover = ColorFromBytes(67, 138, 203);
  inline static constexpr auto Selection = ColorFromBytes(38, 71, 102);
  inline static constexpr auto SelectionInactive = ColorFromBytes(35, 51, 68);
  inline static constexpr auto DockPreview = ColorFromBytes(53, 120, 184, 72);
  inline static constexpr auto DockPreviewBorder =
      ColorFromBytes(67, 138, 203, 230);

  inline static constexpr auto Text = ColorFromBytes(214, 214, 216);
  inline static constexpr auto TextMuted = ColorFromBytes(160, 160, 165);
  inline static constexpr auto TextDisabled = ColorFromBytes(102, 102, 107);
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
bool ParseUIColor(const std::string &text, DirectX::XMFLOAT4 &color);

} // namespace z8
