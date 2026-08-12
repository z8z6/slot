#pragma once

#include "UI/Behavior/ScrollBehavior.h"
#include "UI/Layout/BehaviorNode.h"
#include "UI/Layout/ScrollBarNode.h"

namespace z8::ui {

/**
 * 封装 viewport、content、滚动条和 ScrollBehavior 的可复用滚动容器。
 *
 * 外部子节点统一添加到 ContentHost；Panel 不再了解滚动范围、裁剪或滑块绑定，
 * 列表和树控件也可直接复用该组合节点。
 */
class ScrollNode : public BehaviorNode {
public:
  BaseNode *ViewportNode = nullptr;
  BaseNode *ContentNode = nullptr;
  ScrollBarNode *VerticalScrollBarNode = nullptr;
  RectNode *VerticalScrollThumbNode = nullptr;

  ScrollNode();
  const char *TypeName() const override { return "Scroll"; }
  BaseNode *ContentHost() override { return ContentNode; }
  ScrollBehavior *GetScrollBehavior() { return GetBehavior<ScrollBehavior>(); }
  void SetVerticalBarInsets(float top, float right, float bottom);
};

} // namespace z8::ui
