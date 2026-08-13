#pragma once

#include "UI/Layout/PanelNode.h"
#include "UI/Layout/RectNode.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace z8::ui {
class PanelGroupNode;

/**
 * PanelGroup 的单个可点击标题页。它只保存非拥有的 Group 引用和
 * 稳定页索引，Panel 与标题视觉的生命周期仍归 PanelGroup。
 */
class PanelGroupTabNode final : public RectNode {
public:
  PanelGroupNode *Group = nullptr;
  TextNode *LabelNode = nullptr;
  size_t PanelIndex = 0;

  PanelGroupTabNode(PanelGroupNode &group, size_t panelIndex,
                    const std::string &title);
  EventReply OnMouseDown(MouseMovArgs args) override;
  const char *TypeName() const override { return "PanelGroupTab"; }
};

/**
 * Panel 的页签与 Dock 容器。
 *
 * 启用 Dock 的独立 Panel 会在索引布局树时自动包装为单页 Group；
 * 拖到另一 Group 标题栏才会显式合并页签。组内所有页面占用同一内容
 * 矩形，但只有活动 Panel 可见且可命中，避免重叠 Panel 同时接收输入。
 */
class PanelGroupNode final : public RectNode {
public:
  size_t ActivePanel = 0;
  RectNode *TabBarNode = nullptr;
  BaseNode *PagesNode = nullptr;
  std::vector<PanelNode *> Panels;
  std::vector<PanelGroupTabNode *> Tabs;

  PanelGroupNode();

  bool ActivatePanel(size_t panelIndex);
  BaseNode *AddChild(std::unique_ptr<BaseNode> child) override;
  PanelNode *AddPanel(std::unique_ptr<PanelNode> panel);
  BaseNode *ContentHost() override { return this; }
  /** 即时声明需保留 Group 已由用户拖动或拉伸提交的几何。 */
  bool HasInteractiveGeometry() const;
  /** 把另一组的 Panel 所有权转移到本组，用于标题栏拖放合并。 */
  void MergeFrom(PanelGroupNode &source);
  bool SetProperty(const std::string &name, const std::string &value) override;
  const char *TypeName() const override { return "PanelGroup"; }

private:
  void UpdateSelectionVisuals();
};

} // namespace z8::ui
