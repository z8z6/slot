//
// Created by zhou_zhengming on 2026/7/31.
//

#pragma once


#include "RectNode.h"
#include "UI/Layout/ScrollBarNode.h"
#include "UI/Property/IDraggable.h"
#include "UI/Property/IResizable.h"
#include "UI/Property/IScrollable.h"

#include <string>

namespace z8::ui {
class PanelNode : public RectNode,
                  public IDraggable,
                  public IResizable,
                  public IScrollable {
public:
  RectNode* TitleNode;
  BaseNode* ScrollViewportNode;
  BaseNode* ContentNode;
  ScrollBarNode* VerticalScrollBarNode;
  RectNode* VerticalScrollThumbNode;
  std::string Title;

  PanelNode();
  BaseNode* ContentHost() override;
  const char* TypeName() const override { return "Panel"; }
  bool SetProperty(const std::string& name, const std::string& value) override;
  bool OnMouseDown(MouseMovArgs args) override;
  bool OnMouseDrag(MouseMovArgs args) override;
  bool OnMouseUp(MouseMovArgs args) override;
  bool OnMouseWheel(MouseWheelArgs args) override;
  void OnLayoutUpdated() override;
  MouseCursor GetMouseCursor(MouseMovArgs args) const override;
  /** 即时声明重复提交初始样式时，用它保护用户已经调整的几何。 */
  bool HasInteractiveGeometry() const {
    return HasDragGeometry() || HasResizeGeometry();
  }
  void SetResizeProperties(const ResizeProperty& properties) override;
  void SetScrollProperties(const ScrollProperty& properties) override;

private:
  float TitleHeight = 32.0f;

  void ScrollOffsetChanged(float offset) override;
  void ApplyScrollProperties();
};
} // namespace z8::ui
