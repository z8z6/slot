#pragma once

#include <limits>
#include <optional>

namespace z8::ui {

/** Flex 主轴方向；当前原生布局系统专注于 Windows UI 常用的单行行列布局。 */
enum class FlexDirection { Column, Row };

/** 节点参与普通流或脱离普通流，绝对定位节点不会消耗兄弟节点空间。 */
enum class PositionType { Relative, Absolute };

/**
 * 布局输入样式。
 *
 * optional 表示 auto，而不是用特殊浮点值编码状态，避免 NaN 在约束计算和
 * 调试器中传播。边距和内边距目前采用项目现有 API 的统一四边语义；四个
 * Position 字段只对 Absolute 节点生效。
 */
struct LayoutStyle {
  std::optional<float> Width;
  std::optional<float> Height;
  std::optional<float> WidthPercent;
  std::optional<float> Left;
  std::optional<float> Top;
  std::optional<float> Right;
  std::optional<float> Bottom;
  float MinWidth = 0.0f;
  float MinHeight = 0.0f;
  float MaxWidth = (std::numeric_limits<float>::max)();
  float MaxHeight = (std::numeric_limits<float>::max)();
  float FlexGrow = 1.0f;
  float FlexShrink = 1.0f;
  float Margin = 0.0f;
  float Padding = 0.0f;
  FlexDirection Direction = FlexDirection::Column;
  PositionType Position = PositionType::Relative;
};

/** 布局求解结果始终位于父节点内容坐标系，绝对屏幕坐标由 Layout 统一传播。 */
struct LayoutBox {
  float Left = 0.0f;
  float Top = 0.0f;
  float Width = 0.0f;
  float Height = 0.0f;
};

} // namespace z8::ui
