#pragma once

#include "UI/Dock/DockTree.h"

#include <unordered_map>

namespace z8::ui {
class BaseNode;
class PanelGroupNode;
class PanelNode;

enum class PanelDragState { Idle, Pressed, Dragging };
enum class DragPayloadType { None, Panel, PanelGroup };

/** 一次标题栏拖拽的全部临时状态；其生命周期内 DockTree 保持只读。 */
struct PanelDragSession {
  PanelDragState State = PanelDragState::Idle;
  DragPayloadType PayloadType = DragPayloadType::None;
  PanelNode *PayloadPanel = nullptr;
  PanelGroupNode *PayloadGroup = nullptr;
  PanelGroupNode *SourceGroup = nullptr;
  /**
   * 明确的标题栏操作目标：空白区用于跨组加入，同组已有 Tab 用于交换。
   * 普通 Center Leaf 不设置它，避免把内容区域误解释为页签合并。
   */
  PanelGroupNode *TargetGroup = nullptr;
  /** 参与预览与 DockTree 查询的 Group；Panel 拖动时仍指向来源 Group。 */
  BaseNode *Panel = nullptr;
  DockNodeID SourceNode = 0;
  int SourceTabIndex = -1;
  bool SourceWasFloating = false;
  /** 当前指针位置是否构成有效预览；来源区域使用 Floating 而不是 Dock 预览。 */
  bool PreviewVisible = false;
  DockRect SourceFloatingRect;
  float PressClientX = 0.0f;
  float PressClientY = 0.0f;
  float MouseClientX = 0.0f;
  float MouseClientY = 0.0f;
  float GrabOffsetX = 0.0f;
  float GrabOffsetY = 0.0f;
  DockRect FloatingPreviewRect;
  DockNodeID DockTarget = 0;
  DockSide Side = DockSide::Center;
  /** 同组 Panel 拖动时命中的目标页签；-1 表示空白标题栏合入或非页签目标。 */
  int TargetTabIndex = -1;
  DockRect DockPreviewRect;
};

/** Panel 在 DockTree 之外的稳定归属信息。 */
struct PanelDockState {
  PanelPlacement Placement = PanelPlacement::Docked;
  DockNodeID DockNode = 0;
  DockRect FloatingRect;
};

/**
 * 单个 Layout 的 Dock 上下文，统一管理 Docked/Floating 互斥关系和 DragSession。
 * Client 坐标与 DockRect 均为窗口客户区 UI 像素，禁止在此层混入屏幕坐标。
 */
class DockWorkspace final {
public:
  DockTree Tree;
  PanelDragSession Drag;

  void ApplyLayout(float width, float height);
  void BeginDrag(BaseNode &panel, float clientX, float clientY);
  void BeginPanelDrag(PanelNode &panel, PanelGroupNode &sourceGroup,
                      size_t sourceTabIndex, float clientX, float clientY);
  void CancelDrag();
  bool CommitDrag(float clientX, float clientY);
  DockSide DetectSide(const DockRect &rect, float clientX, float clientY) const;
  const PanelDockState *GetState(const BaseNode &panel) const;
  /** 返回当前所有 Floating Group；调用者不得通过结果转移节点所有权。 */
  std::vector<BaseNode *> GetFloatingPanels() const;
  bool IsDocked(const BaseNode &panel) const;
  bool IsFloating(const BaseNode &panel) const;
  /** 为 Commit 阶段新建的单页 Group 建立唯一 Dock/Floating 归属。 */
  bool PlaceNew(BaseNode &group, DockNodeID target, DockSide side,
                const DockRect &floatingRect);
  void Reconcile(const std::vector<BaseNode *> &nodes);
  /** 在控件所有权释放前清除 Group 的 Dock/Floating 唯一归属。 */
  bool Remove(BaseNode &panel);
  void UpdateDrag(float clientX, float clientY);
  /** 原生浮动宿主调整尺寸后回写唯一 FloatingRect，不修改 DockTree。 */
  bool UpdateFloatingRect(BaseNode &panel, const DockRect &rect);
  bool ResizeSplitter(DockNodeID split, float clientX, float clientY);
  std::string DumpDebug() const;
  /** 校验 Docked/Floating 互斥与树内 Panel 唯一归属。 */
  bool Validate(std::string *error = nullptr) const;

private:
  std::unordered_map<BaseNode *, PanelDockState> States;
  std::vector<BaseNode *> PendingInitialPanels;
  /** 最近一次结构提交的前后快照，用于复现拖放错误。 */
  std::string LastTransactionTrace;

  void ApplyPanelRect(BaseNode &panel, const DockRect &rect, bool visible);
  void BuildInitialTree(const std::vector<BaseNode *> &panels, float width,
                        float height);
};

} // namespace z8::ui
