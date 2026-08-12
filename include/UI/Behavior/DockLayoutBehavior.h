#pragma once

#include "UI/Behavior/IBehavior.h"

namespace z8::ui {

/**
 * 为直接子节点分配停靠空间的容器行为。
 *
 * 声明式边缘节点依顺序按 Extent 切割；用户交互产生的同轴停靠槽与
 * Auto/Fill 节点均分该轴空间，Floating 节点保持用户几何。当前边界限定为直接子节点，避免父子
 * DockSpace 在同一 Yoga 计算前使用尚未更新的嵌套尺寸。
 */
class DockLayoutBehavior final : public IBehavior {
public:
  static constexpr int DefaultPriority = 300;

  DockLayoutBehavior() : IBehavior(DefaultPriority) {}
  /** 在 Yoga 测量前把容器可用矩形写入所有参与停靠的直接子节点。 */
  void OnBeforeLayout(float width, float height) override;

private:
  /** 把一个父空间矩形转换为无 Margin 的 Yoga 绝对布局约束。 */
  static void ApplyGeometry(BaseNode *node, float left, float top, float width,
                            float height);
};

} // namespace z8::ui
