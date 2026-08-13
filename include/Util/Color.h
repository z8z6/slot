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
  // 蓝黑编辑器层级取自参考界面的视觉关系：背景偏冷，抬升层只增加少量
  // 明度，让长时间使用保持低眩光，同时仍能辨认 Panel 与输入控件边界。
  inline static constexpr auto EditorBackground = ColorFromBytes(11, 16, 22);
  inline static constexpr auto PanelBackground = ColorFromBytes(16, 22, 29);
  inline static constexpr auto HeaderBackground = ColorFromBytes(22, 29, 38);
  inline static constexpr auto ControlBackground = ColorFromBytes(20, 27, 35);
  inline static constexpr auto ControlBackgroundAlt = ColorFromBytes(17, 23, 30);
  inline static constexpr auto ControlHover = ColorFromBytes(30, 41, 54);
  inline static constexpr auto ControlPressed = ColorFromBytes(13, 19, 26);
  inline static constexpr auto Border = ColorFromBytes(43, 55, 68);
  inline static constexpr auto Divider = ColorFromBytes(49, 61, 74);

  // UE 编辑器常用的蓝色选择语义；弱强调用于行底色，强强调用于焦点和选中。
  inline static constexpr auto Accent = ColorFromBytes(42, 139, 255);
  inline static constexpr auto AccentHover = ColorFromBytes(73, 160, 255);
  inline static constexpr auto Selection = ColorFromBytes(25, 76, 132);
  inline static constexpr auto SelectionInactive = ColorFromBytes(29, 53, 78);

  inline static constexpr auto Text = ColorFromBytes(218, 225, 234);
  inline static constexpr auto TextMuted = ColorFromBytes(157, 169, 184);
  inline static constexpr auto TextDisabled = ColorFromBytes(91, 103, 117);
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
