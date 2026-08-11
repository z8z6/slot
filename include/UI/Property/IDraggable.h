#pragma once

#include "UI/Property/IProperty.h"

namespace z8::ui {

/** 控件允许从标题栏或任意内部区域启动移动。 */
enum class DragRegion { TitleBar, Anywhere };

/** 可序列化的拖拽配置；运行时手势状态由 DragBehavior 维护。 */
struct DragProperty {
  bool Enabled = true;
  DragRegion Region = DragRegion::TitleBar;
};

/**
 * 可拖拽能力的兼容查询接口。
 *
 * 新代码应通过 BaseNode::AddBehavior<DragBehavior>() 组合能力。该接口不再
 * 保存状态，仅让旧的检查器和调用方在迁移期间查询或修改行为配置。
 */
class IDraggable : public virtual IProperty {
public:
  ~IDraggable() override = default;
  virtual const DragProperty &GetDragProperties() const = 0;
  virtual void SetDragProperties(const DragProperty &properties) = 0;
  virtual bool IsDragging() const = 0;
  virtual bool HasDragGeometry() const = 0;
};

} // namespace z8::ui
