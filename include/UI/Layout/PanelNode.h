//
// Created by zhou_zhengming on 2026/7/31.
//

#pragma once

#include "RectNode.h"
#include "UI/Behavior/DockBehavior.h"
#include "UI/Behavior/DragBehavior.h"
#include "UI/Behavior/ResizeBehavior.h"
#include "UI/Layout/ImageNode.h"
#include "UI/Layout/ScrollNode.h"
#include "UI/Layout/TextNode.h"

#include <string>

namespace z8::ui {
class PanelGroupNode;
class PanelNode : public RectNode {
public:
  /** 所属页签组；非拥有指针，未入组的独立 Panel 为空。 */
  PanelGroupNode *Group = nullptr;
  /** 标题图标的资源 URI；入组后由对应 Tab 继续展示同一图标。 */
  std::string IconSource;
  ImageNode *TitleIconNode;
  RectNode *TitleBarNode;
  TextNode *TitleNode;
  ScrollNode *ScrollAreaNode;

  PanelNode();
  BaseNode *ContentHost() override;
  /** 即时声明重复提交初始样式时，用它保护用户已经调整的几何。 */
  bool HasInteractiveGeometry() const {
    const auto *resize = GetBehavior<ResizeBehavior>();
    return resize && resize->HasInteractiveGeometry();
  }
  bool SetProperty(const std::string &name, const std::string &value) override;
  /** 内部编辑器控件可按 UIIcon 设置标题图标，声明层仍可继续使用 Source URI。 */
  bool SetTitleIcon(UIIcon icon);
  const char *TypeName() const override { return "Panel"; }

private:
  /** 构造时取自 Theme；实例属性可覆盖但不会修改全局主题。 */
  float TitleHeight = 0.0f;
};
} // namespace z8::ui
