#pragma once

#include "UI/Behavior/UIBehavior.h"

namespace z8::ui {

/**
 * 为直接子节点分配停靠空间的容器行为。
 *
 * 边缘节点依声明顺序从可用矩形中切割空间，Auto/Fill 节点再横向均分剩余
 * 区域，Floating 节点保持用户几何。当前边界限定为直接子节点，避免父子
 * DockSpace 在同一 Yoga 计算前使用尚未更新的嵌套尺寸。
 */
class DockLayoutBehavior final : public UIBehavior {
public:
  static constexpr int DefaultPriority = 300;

  DockLayoutBehavior() : UIBehavior(DefaultPriority) {}
  /** 在 Yoga 测量前把容器可用矩形写入所有参与停靠的直接子节点。 */
  void OnBeforeLayout(float width, float height) override;

private:
  /** 把一个父空间矩形转换为无 Margin 的 Yoga 绝对布局约束。 */
  static void ApplyGeometry(BaseNode *node, float left, float top, float width,
                            float height);
};

} // namespace z8::ui
