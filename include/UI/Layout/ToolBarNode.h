#pragma once

#include "UI/Layout/RectNode.h"

namespace z8::ui {

/** 固定在工作区顶部的紧凑 Menu 集合；不创建标题栏、页签或滚动内容区。 */
class ToolBarNode final : public RectNode {
public:
  ToolBarNode();

  BaseNode *AddChild(std::unique_ptr<BaseNode> child) override;
  const char *TypeName() const override { return "ToolBar"; }
};

} // namespace z8::ui
