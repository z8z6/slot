#pragma once

#include "UI/Behavior/IBehavior.h"

namespace z8::ui {

/** 初次构建 DockTree 时使用的声明策略；运行时真值位于 DockTree。 */
enum class DockPlacement { Auto, Left, Right, Top, Bottom, Fill };

/**
 * 单个可停靠节点的配置。
 *
 * Extent 是边缘停靠时沿切割轴占用的逻辑像素；Auto/Fill 由容器均分剩余空间。
 */
struct DockProperty {
  bool Enabled = true;
  /** false 表示边缘控件保持 Extent 像素，并且不暴露相邻 Splitter。 */
  bool Resizable = true;
  DockPlacement Placement = DockPlacement::Auto;
  float EdgeThreshold = 48.0f;
  float Extent = 300.0f;
};

/**
 * 标记一个 BehaviorNode 可被 DockWorkspace 管理，并解析初始声明配置。
 * 组件本身不修改树或几何，避免输入策略成为布局真值。
 */
class DockBehavior final : public IBehavior {
public:
  DockProperty Properties;

  /** 解析声明式停靠配置；无法识别的名称继续交给其他行为。 */
  bool SetProperty(const std::string &name, const std::string &value) override;
  /** 拖动只表达重排意图，结构事务由 DockWorkspace 在释放时提交。 */
  void OnDragStarted(MouseMovArgs args) override;
  void OnDragCompleted(MouseMovArgs args) override;
};

} // namespace z8::ui
