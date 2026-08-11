#pragma once

#include "Core/Event.h"
#include "UI/Property/IProperty.h"

namespace z8::ui {

class BaseNode;

/** 控件允许从标题栏或任意内部区域启动移动。 */
enum class DragRegion { TitleBar, Anywhere };

/** 可序列化的拖拽配置；运行时手势状态由 IDraggable 内部维护。 */
struct DragProperty {
  bool Enabled = true;
  DragRegion Region = DragRegion::TitleBar;
};

/**
 * 可拖拽能力 mixin。
 *
 * 派生控件只判断本次按下是否位于允许区域，坐标快照、Yoga 绝对定位转换和
 * 捕获期间的连续位移统一由该能力维护，避免每种浮动控件重复实现状态机。
 */
class IDraggable : public virtual IProperty {
public:
  virtual const DragProperty& GetDragProperties() const {
    return DragProperties;
  }
  virtual void SetDragProperties(const DragProperty& properties) {
    DragProperties = properties;
  }
  bool IsDragging() const { return Dragging; }
  bool HasDragGeometry() const { return InteractiveGeometry; }

protected:
  bool BeginDrag(BaseNode* node, MouseMovArgs args, bool inAllowedRegion);
  bool UpdateDrag(BaseNode* node, MouseMovArgs args);
  bool EndDrag();

private:
  DragProperty DragProperties;
  bool Dragging = false;
  bool InteractiveGeometry = false;
  float CurrentLeft = 0.0f;
  float CurrentTop = 0.0f;
  float CurrentWidth = 0.0f;
  float CurrentHeight = 0.0f;
};

} // namespace z8::ui
