#pragma once

#include "UI/Behavior/IBehavior.h"

namespace z8::ui {

/** Panel 相对停靠容器的布局策略；Floating 保留用户拖动后的绝对几何。 */
enum class DockPlacement { Auto, Floating, Left, Right, Top, Bottom, Fill };

/**
 * 单个可停靠节点的配置。
 *
 * Extent 是边缘停靠时沿切割轴占用的逻辑像素；Auto/Fill 由容器均分剩余空间。
 */
struct DockProperty {
  bool Enabled = true;
  DockPlacement Placement = DockPlacement::Auto;
  float EdgeThreshold = 48.0f;
  float Extent = 300.0f;
  // 交互停靠使用均分槽位，声明式 DockExtent 仍保留固定工具栏/侧栏语义。
  bool EqualShare = false;
};

/**
 * 把 DragBehavior 的手势结果转换为停靠状态。
 *
 * 该行为不修改布局几何：拖动开始时仅切换为 Floating，释放到父容器边缘时
 * 记录新的 Placement；实际空间分配由父节点的 DockLayoutBehavior 完成。
 */
class DockBehavior final : public IBehavior {
public:
  DockProperty Properties;

  /** 解析声明式停靠配置；无法识别的名称继续交给其他行为。 */
  bool SetProperty(const std::string &name, const std::string &value) override;
  /** 拖动发生真实位移后解除布局约束，释放时根据最近父边缘重新停靠。 */
  void OnDragStarted(MouseMovArgs args) override;
  void OnDragCompleted(MouseMovArgs args) override;
};

} // namespace z8::ui
