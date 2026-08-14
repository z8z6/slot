#pragma once

#include "UI/Layout/ImageNode.h"
#include "UI/Layout/PanelNode.h"
#include "UI/Layout/RectNode.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace z8::ui {
class PanelGroupNode;

/** Group 标题栏关闭按钮；只发出关闭请求，所有权回收由 Layout 统一提交。 */
class PanelGroupCloseNode final : public RectNode {
public:
  PanelGroupNode *Group = nullptr;
  ImageNode *IconNode = nullptr;

  explicit PanelGroupCloseNode(PanelGroupNode &group);
  EventReply OnMouseDown(MouseMovArgs args) override;
  const char *TypeName() const override { return "PanelGroupClose"; }

private:
  void OnVisualStateChanged() override;
};

/**
 * PanelGroup 的单个可点击标题页。它只保存非拥有的 Group 引用和
 * 稳定页索引，Panel 与标题视觉的生命周期仍归 PanelGroup。
 */
class PanelGroupTabNode final : public RectNode {
public:
  PanelGroupNode *Group = nullptr;
  ImageNode *IconNode = nullptr;
  TextNode *LabelNode = nullptr;
  RectNode *SelectionNode = nullptr;
  size_t PanelIndex = 0;

  PanelGroupTabNode(PanelGroupNode &group, size_t panelIndex,
                    const std::string &title, const std::string &iconSource);
  EventReply OnMouseDown(MouseMovArgs args) override;
  /** Active 变化时重新解析与 Hover/Pressed 正交的页签视觉状态。 */
  void RefreshVisualState();
  /** 更新页签图标，同时保留 ImageNode 的资源协议校验。 */
  bool SetIcon(const std::string &iconSource);
  /** 内部工具控件使用强类型图标，避免把资源 URI 散落到业务代码。 */
  bool SetIcon(UIIcon icon);
  /** 标题变化时同步更新固有宽度，避免复用页签保留旧标题尺寸。 */
  void SetTitle(const std::string &title);
  const char *TypeName() const override { return "PanelGroupTab"; }

private:
  void OnVisualStateChanged() override;
};

/**
 * Panel 的页签与 Dock 容器。
 *
 * 启用 Dock 的独立 Panel 会在索引布局树时自动包装为单页 Group。
 * Dock 拖动始终操作整个 Group，Tab 只负责切换页面。组内所有页面占用
 * 同一内容矩形，但只有活动 Panel 可见且可命中，避免重叠输入。
 */
class PanelGroupNode final : public RectNode {
public:
  size_t ActivePanel = 0;
  bool CloseRequested = false;
  PanelGroupCloseNode *CloseButtonNode = nullptr;
  RectNode *ContentSeparatorNode = nullptr;
  RectNode *DragHandleNode = nullptr;
  RectNode *HeaderNode = nullptr;
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
  /** 移除指定页并选择相邻活动页；空 Group 会请求由 Layout 回收。 */
  std::unique_ptr<PanelNode> RemovePanel(size_t panelIndex);
  bool SetProperty(const std::string &name, const std::string &value) override;
  /** 交换两个页签及其 Panel 的排列位置，同时保持活动 Panel 身份不变。 */
  bool SwapPanels(size_t firstIndex, size_t secondIndex);
  const char *TypeName() const override { return "PanelGroup"; }

private:
  void UpdateSelectionVisuals();
};

} // namespace z8::ui
