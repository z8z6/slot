//
// Created by zhou_zhengming on 2026/7/29.
//

#pragma once
#include "Core/Event.h"
#include "UI/Dock/DockWorkspace.h"
#include <DirectXMath.h>

#include <memory>
#include <string>
#include <vector>

namespace z8 {
class Application;
class GameObject;
} // namespace z8

namespace z8::ui {
class BaseNode;
class BehaviorNode;
class DrawNode;
class TextNode;
class SceneNode;
class TerminalNode;
class RectNode;

/**
 * UI 树的布局与输入路由器。
 */
class Layout : public EventTarget {
public:
  std::unique_ptr<BaseNode> Root;
  std::vector<BaseNode *> Nodes;
  std::vector<DrawNode *> Visuals;
  std::vector<TextNode *> Texts;
  std::vector<SceneNode *> Scenes;
  std::vector<TerminalNode *> Terminals;
  /** 当前窗口的 Dock 结构、浮动归属和唯一拖拽会话。 */
  DockWorkspace Dock;
  /** 拖动候选覆盖层独立于 DockTree，渲染后不参与命中或布局决策。 */
  std::unique_ptr<RectNode> DockPreviewVisual;
  /** 数字键 3 控制的 Panel 边界诊断开关。 */
  bool DebugPanelBorders = false;

  bool Dirty = true;
  BaseNode *CapturedTarget = nullptr;
  BehaviorNode *CapturedHandler = nullptr;
  DockNodeID CapturedSplitter = 0;
  /** 当前唯一 Hover/Pressed 控件；非拥有指针只在本轮稳定控件树内有效。 */
  BehaviorNode *HoveredHandler = nullptr;
  BehaviorNode *PressedHandler = nullptr;
  BehaviorNode *FocusedHandler = nullptr;

  explicit Layout();
  ~Layout() override;

  void RebuildIndex();
  BaseNode *Find(const std::string &key) const;
  void Calculate(float width, float height);
  /** 原生 Floating HWND 激活时同步画家顺序和输入命中优先级。 */
  void ActivateFloating(BaseNode &node);
  void MarkDirty() { Dirty = true; }
  bool ConsumeDirty();
  void SetRoot(std::unique_ptr<BaseNode> root);
  std::vector<GameObject *> GetUO() const;
  /** 主 HWND 只绘制 DockTree 内容；Floating 子树由各自原生宿主呈现。 */
  std::vector<GameObject *> GetMainUO() const;
  /** 返回指定节点完整子树的视觉对象，供原生 Floating 宿主建立批次。 */
  std::vector<GameObject *> GetSubtreeUO(const BaseNode &root) const;
  std::vector<TextNode *> GetMainTexts() const;
  std::vector<TextNode *> GetSubtreeTexts(const BaseNode &root) const;
  /** 返回第一个可见场景视口；当前 DX12 后端以单场景相机渲染该视口。 */
  SceneNode *GetSceneNode() const;
  /** 把运行时消息广播给当前布局中的所有 TerminalNode。 */
  void WriteTerminal(const std::string &message) const;
  /** 返回当前 Dock 或浮动候选预览；Idle 时返回空矩形。 */
  DockRect GetDockPreview() const;
  /** 返回 Panel 子树中当前参与绘制的红色节点边界数量。 */
  size_t GetPanelDebugBorderCount() const { return PanelDebugVisuals.size(); }

  EventReply OnKeyDown(KeyArgs args) override;
  EventReply OnKeyUp(KeyArgs args) override;
  EventReply OnTextInput(wchar_t character) override;
  EventReply OnMouseDown(MouseMovArgs args) override;
  EventReply OnMouseMove(MouseMovArgs args) override;
  EventReply OnMouseDrag(MouseMovArgs args) override;
  EventReply OnMouseUp(MouseMovArgs args) override;
  EventReply OnMouseWheel(MouseWheelArgs args) override;
  MouseCursor GetMouseCursor(MouseMovArgs args) const override;
  void OnPointerCaptureLost() override;

private:
  /** 逐帧复用的非拥有遮挡索引，避免菜单打开期间为每个文字反复分配。 */
  std::vector<DrawNode *> LaterTextOccluders;
  std::vector<std::unique_ptr<RectNode>> PanelDebugVisuals;

  void BringToFront(BaseNode *node);
  void CancelTreeCaptures(BaseNode *node);
  /** 点击菜单层级外时关闭所有顶层弹出目录，不改变稳定节点拓扑。 */
  void CloseMenusOutside(BaseNode *target);
  /** 按统一 DragSession 提交 Panel 成员迁移，并在需要时创建单页 Group。 */
  bool CommitPanelDrop(float clientX, float clientY);
  BehaviorNode *FindBehaviorNode(BaseNode *node) const;
  BaseNode *HitAt(float x, float y) const;
  void IndexTree(BaseNode *node);
  /** 把未分组 Panel 包装为单页 PanelGroup，Dock 树只管理 Group。 */
  void NormalizePanelGroups(BaseNode &parent);
  /** 拓扑更新后从仍存活的树清除短期视觉状态，避免复用节点残留 Hover。 */
  void ResetInteractionStates(BaseNode *node);
  /** 回收请求关闭或已无页面的 Group，并同步清除 Dock/Floating 归属。 */
  bool RemoveClosedPanelGroups(BaseNode &parent);
  /** 根据最终屏幕坐标刷新 Panel 子树诊断覆盖层，不修改节点自身主题状态。 */
  void UpdatePanelDebugVisuals();
  /** 按画家顺序计算后续不透明浮层对 DirectWrite 文字的裁剪片段。 */
  void UpdateTextOcclusion();
  void UpdateTree(BaseNode &node, float parentX, float parentY,
                  const DirectX::XMFLOAT4 &clip, bool dispatchAfterLayout,
                  bool parentVisible = true);
  void UpdateDockPreview();
  /** 根据与渲染一致的最上层命中结果切换唯一 Hover 控件。 */
  void UpdateHoveredHandler(float x, float y);
  /** 切换唯一键盘焦点并同步控件视觉状态。 */
  void SetFocusedHandler(BehaviorNode *handler);

};
} // namespace z8::ui
