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

  bool Dirty = true;
  BaseNode *CapturedTarget = nullptr;
  BehaviorNode *CapturedHandler = nullptr;
  DockNodeID CapturedSplitter = 0;

  explicit Layout();
  ~Layout() override;

  void RebuildIndex();
  BaseNode *Find(const std::string &key) const;
  void Calculate(float width, float height);
  void MarkDirty() { Dirty = true; }
  bool ConsumeDirty();
  void SetRoot(std::unique_ptr<BaseNode> root);
  std::vector<GameObject *> GetUO() const;
  /** 返回第一个可见场景视口；当前 DX12 后端以单场景相机渲染该视口。 */
  SceneNode *GetSceneNode() const;
  /** 把运行时消息广播给当前布局中的所有 TerminalNode。 */
  void WriteTerminal(const std::string &message) const;
  /** 返回当前 Dock 或浮动候选预览；Idle 时返回空矩形。 */
  DockRect GetDockPreview() const;

  EventReply OnMouseDown(MouseMovArgs args) override;
  EventReply OnMouseMove(MouseMovArgs args) override;
  EventReply OnMouseDrag(MouseMovArgs args) override;
  EventReply OnMouseUp(MouseMovArgs args) override;
  EventReply OnMouseWheel(MouseWheelArgs args) override;
  MouseCursor GetMouseCursor(MouseMovArgs args) const override;
  void OnPointerCaptureLost() override;

private:
  void BringToFront(BaseNode *node);
  void CancelTreeCaptures(BaseNode *node);
  void CommitPanelGroupMerge();
  BehaviorNode *FindBehaviorNode(BaseNode *node) const;
  BaseNode *HitAt(float x, float y) const;
  void IndexTree(BaseNode *node);
  /** 把未分组 Panel 包装为单页 PanelGroup，Dock 树只管理 Group。 */
  void NormalizePanelGroups(BaseNode &parent);
  void UpdateTree(BaseNode &node, float parentX, float parentY,
                  const DirectX::XMFLOAT4 &clip, bool dispatchAfterLayout,
                  bool parentVisible = true);
  void UpdateDockPreview();
};
} // namespace z8::ui
