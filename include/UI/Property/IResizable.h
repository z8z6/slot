#pragma once

#include "Core/Event.h"
#include "UI/Property/IProperty.h"

namespace z8::ui {

class BaseNode;

/** 拉伸命中区域；角落同时改变两个轴。 */
enum class ResizeRegion {
  None,
  Left,
  Right,
  Top,
  Bottom,
  TopLeft,
  TopRight,
  BottomLeft,
  BottomRight
};

/** 可序列化的拉伸配置，边界宽度使用窗口客户区逻辑像素。 */
struct ResizeProperty {
  bool Enabled = true;
  float Border = 6.0f;
  float MinWidth = 120.0f;
  float MinHeight = 80.0f;
};

/** 可拉伸能力 mixin，负责命中、光标、捕获状态和最小尺寸约束。 */
class IResizable : public virtual IProperty {
public:
  virtual const ResizeProperty& GetResizeProperties() const {
    return ResizeProperties;
  }
  virtual void SetResizeProperties(const ResizeProperty& properties) {
    ResizeProperties = properties;
  }
  ResizeRegion HitTestResize(const BaseNode* node, MouseMovArgs args) const;
  MouseCursor GetResizeCursor(const BaseNode* node, MouseMovArgs args) const;
  bool IsResizing() const { return ActiveRegion != ResizeRegion::None; }
  bool HasResizeGeometry() const { return InteractiveGeometry; }

protected:
  bool BeginResize(BaseNode* node, MouseMovArgs args);
  bool UpdateResize(BaseNode* node, MouseMovArgs args);
  bool EndResize();

private:
  static MouseCursor CursorForRegion(ResizeRegion region);

  ResizeProperty ResizeProperties;
  ResizeRegion ActiveRegion = ResizeRegion::None;
  bool InteractiveGeometry = false;
  float CurrentLeft = 0.0f;
  float CurrentTop = 0.0f;
  float CurrentWidth = 0.0f;
  float CurrentHeight = 0.0f;
};

} // namespace z8::ui
