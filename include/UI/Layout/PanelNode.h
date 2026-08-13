//
// Created by zhou_zhengming on 2026/7/31.
//

#pragma once

#include "RectNode.h"
#include "UI/Behavior/DockBehavior.h"
#include "UI/Behavior/DragBehavior.h"
#include "UI/Behavior/ResizeBehavior.h"
#include "UI/Layout/ScrollNode.h"
#include "UI/Layout/TextNode.h"

#include <string>

namespace z8::ui {
class PanelGroupNode;
class PanelNode : public RectNode {
public:
  /** 所属页签组；非拥有指针，未入组的独立 Panel 为空。 */
  PanelGroupNode *Group = nullptr;
  RectNode *TitleBarNode;
  TextNode *TitleNode;
  ScrollNode *ScrollAreaNode;

  PanelNode();
  BaseNode *ContentHost() override;
  const char *TypeName() const override { return "Panel"; }
  bool SetProperty(const std::string &name, const std::string &value) override;
  /** 即时声明重复提交初始样式时，用它保护用户已经调整的几何。 */
  bool HasInteractiveGeometry() const {
    const auto *resize = GetBehavior<ResizeBehavior>();
    return resize && resize->HasInteractiveGeometry();
  }

private:
  float TitleHeight = 32.0f;
};
} // namespace z8::ui
