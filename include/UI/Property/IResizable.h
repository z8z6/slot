#pragma once

#include "UI/Property/IProperty.h"

namespace z8::ui {

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

/**
 * 可拉伸能力的兼容查询接口；实际命中和手势状态由 ResizeBehavior 独占。
 */
class IResizable : public virtual IProperty {
public:
  ~IResizable() override = default;
  virtual const ResizeProperty &GetResizeProperties() const = 0;
  virtual void SetResizeProperties(const ResizeProperty &properties) = 0;
  virtual bool IsResizing() const = 0;
  virtual bool HasResizeGeometry() const = 0;
};

} // namespace z8::ui
