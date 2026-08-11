//
// Created by zhou_zhengming on 2026/7/31.
//

#pragma once

#include "RectNode.h"
#include "UI/Behavior/DragBehavior.h"
#include "UI/Behavior/ResizeBehavior.h"
#include "UI/Behavior/ScrollBehavior.h"
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
  RectNode *TitleNode;
  BaseNode *ScrollViewportNode;
  BaseNode *ContentNode;
  ScrollBarNode *VerticalScrollBarNode;
  RectNode *VerticalScrollThumbNode;
  std::string Title;

  PanelNode();
  BaseNode *ContentHost() override;
  const char *TypeName() const override { return "Panel"; }
  bool SetProperty(const std::string &name, const std::string &value) override;
  /** 即时声明重复提交初始样式时，用它保护用户已经调整的几何。 */
  bool HasInteractiveGeometry() const {
    return HasDragGeometry() || HasResizeGeometry();
  }
  const DragProperty &GetDragProperties() const override;
  void SetDragProperties(const DragProperty &properties) override;
  bool IsDragging() const override;
  bool HasDragGeometry() const override;
  const ResizeProperty &GetResizeProperties() const override;
  void SetResizeProperties(const ResizeProperty &properties) override;
  bool IsResizing() const override;
  bool HasResizeGeometry() const override;
  const ScrollProperty &GetScrollProperties() const override;
  void SetScrollProperties(const ScrollProperty &properties) override;
  float GetScrollOffsetY() const override;
  float GetMaximumScrollOffsetY() const override;
  void SetScrollOffsetY(float offset) override;

private:
  float TitleHeight = 32.0f;
  // 兼容接口只转发到这些独立组件；Panel 不再持有任何手势状态机。
  DragBehavior *DragController = nullptr;
  ResizeBehavior *ResizeController = nullptr;
  ScrollBehavior *ScrollController = nullptr;
};
} // namespace z8::ui
