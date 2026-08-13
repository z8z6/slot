//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/Layout.h"

#include "Object/UIObject/UIObject.h"
#include "UI/Behavior/DockBehavior.h"
#include "UI/Behavior/DragBehavior.h"
#include "UI/Behavior/ResizeBehavior.h"
#include "UI/Layout/BaseNode.h"
#include "UI/Layout/BehaviorNode.h"
#include "UI/Layout/DrawNode.h"
#include "UI/Layout/LayoutEngine.h"
#include "UI/Layout/PanelGroupNode.h"
#include "UI/Layout/PanelNode.h"
#include "UI/Layout/RectNode.h"
#include "UI/Layout/SceneNode.h"
#include "UI/Layout/TextNode.h"
#include "UI/Layout/TerminalNode.h"

#include <algorithm>
#include <ostream>

using namespace z8;
using namespace z8::ui;

namespace {

std::string InteractionName(const BehaviorNode &node) {
  if (const auto *group = dynamic_cast<const PanelGroupNode *>(&node);
      group && !group->Panels.empty()) {
    const auto *panel = group->Panels[group->ActivePanel];
    return panel->Key.empty() ? panel->TypeName() : panel->Key;
  }
  return node.Key.empty() ? node.TypeName() : node.Key;
}

bool IsMinimumBlocked(const ResizeBehavior &resize, const BehaviorNode &node,
                      const MouseMovArgs &args) {
  const auto region = resize.GetActiveRegion();
  const bool widthAtMinimum =
      node.Width <= resize.Properties.MinWidth + 0.01f;
  const bool heightAtMinimum =
      node.Height <= resize.Properties.MinHeight + 0.01f;
  const bool blocksWidth =
      ((region == ResizeRegion::Left || region == ResizeRegion::TopLeft ||
        region == ResizeRegion::BottomLeft) && args.DeltaX > 0) ||
      ((region == ResizeRegion::Right || region == ResizeRegion::TopRight ||
        region == ResizeRegion::BottomRight) && args.DeltaX < 0);
  const bool blocksHeight =
      ((region == ResizeRegion::Top || region == ResizeRegion::TopLeft ||
        region == ResizeRegion::TopRight) && args.DeltaY > 0) ||
      ((region == ResizeRegion::Bottom || region == ResizeRegion::BottomLeft ||
        region == ResizeRegion::BottomRight) && args.DeltaY < 0);
  return (widthAtMinimum && blocksWidth) ||
         (heightAtMinimum && blocksHeight);
}

} // namespace

Layout::Layout(): Root(std::make_unique<BehaviorNode>()) {
  Root->Key = "Root";
  DockPreviewVisual = std::make_unique<RectNode>();
  DockPreviewVisual->Key = "__dock_preview";
  DockPreviewVisual->HitTestVisible = false;
  DockPreviewVisual->Visible = false;
  DockPreviewVisual->EffectiveVisible = false;
  DockPreviewVisual->SetColor({0.1f, 0.45f, 0.9f, 0.28f});
  DockPreviewVisual->SetBorder({0.25f, 0.65f, 1.0f, 0.9f}, 2.0f);
  RebuildIndex();
}

Layout::~Layout() = default;

void Layout::ActivateFloating(BaseNode &node) { BringToFront(&node); }

void Layout::SetRoot(std::unique_ptr<BaseNode> root) {
  Root = std::move(root);
  RebuildIndex();
}

void Layout::IndexTree(BaseNode *node) {
  if (!node)
    return;
  Nodes.push_back(node);
  // RTTI 只在拓扑变化时使用；布局热路径直接调用窄虚接口。
  if (auto *visual = dynamic_cast<DrawNode *>(node))
    Visuals.push_back(visual);
  if (auto *text = dynamic_cast<TextNode *>(node))
    Texts.push_back(text);
  if (auto *scene = dynamic_cast<SceneNode *>(node))
    Scenes.push_back(scene);
  if (auto *terminal = dynamic_cast<TerminalNode *>(node))
    Terminals.push_back(terminal);
  for (const auto &child : node->Children)
    IndexTree(child.get());
}

void Layout::CancelTreeCaptures(BaseNode *node) {
  if (!node)
    return;
  // 从当前仍存活的树遍历，而不是解引用旧索引中的捕获指针；声明式协调可能
  // 已经删除旧 target，但被保留节点上的状态机仍需要收到 capture-lost。
  if (auto *behavior = dynamic_cast<BehaviorNode *>(node))
    behavior->CancelPointerCapture();
  for (const auto &child : node->Children)
    CancelTreeCaptures(child.get());
}

void Layout::RebuildIndex() {
  // 声明式重建可能销毁旧节点，不能让指针捕获跨越拓扑变化。
  CancelTreeCaptures(Root.get());
  RemoveClosedPanelGroups(*Root);
  NormalizePanelGroups(*Root);
  CapturedTarget = nullptr;
  CapturedHandler = nullptr;
  Nodes.clear();
  Visuals.clear();
  Texts.clear();
  Scenes.clear();
  Terminals.clear();
  IndexTree(Root.get());
  std::vector<BaseNode *> dockPanels;
  for (const auto &child : Root->Children) {
    auto *behavior = dynamic_cast<BehaviorNode *>(child.get());
    if (behavior && behavior->GetBehavior<DockBehavior>())
      dockPanels.push_back(child.get());
  }
  Dock.Reconcile(dockPanels);
  Dirty = true;
}

BehaviorNode *Layout::FindBehaviorNode(BaseNode *node) const {
  for (; node; node = node->Parent)
    if (auto *behavior = dynamic_cast<BehaviorNode *>(node))
      return behavior;
  return nullptr;
}

// 返回命中的最上层交互节点；纯布局和文字节点不会意外吞掉场景输入。
BaseNode *Layout::HitAt(float x, float y) const {
  // Nodes 按树的前序遍历，逆序对应画家顺序的最上层优先。
  for (auto iterator = Nodes.rbegin(); iterator != Nodes.rend(); ++iterator) {
    auto *behavior = dynamic_cast<BehaviorNode *>(*iterator);
    if (behavior && behavior->HitTestVisible && behavior->Contains(x, y))
      return behavior;
  }
  return nullptr;
}

z8::MouseCursor Layout::GetMouseCursor(MouseMovArgs args) const {
  if (CapturedSplitter) {
    const auto *split = Dock.Tree.Find(CapturedSplitter);
    return split && split->Axis == SplitAxis::Horizontal
               ? MouseCursor::SizeVertical
               : MouseCursor::SizeHorizontal;
  }
  if (const auto *split = Dock.Tree.FindSplitterAt(
          static_cast<float>(args.X), static_cast<float>(args.Y)))
    return split->Axis == SplitAxis::Horizontal ? MouseCursor::SizeVertical
                                                : MouseCursor::SizeHorizontal;
  if (CapturedHandler) {
    const auto cursor = CapturedHandler->QueryMouseCursor(args);
    if (cursor != MouseCursor::Arrow)
      return cursor;
  }
  for (auto *node = HitAt(static_cast<float>(args.X),
                            static_cast<float>(args.Y));
       node;
       node = node->Parent) {
    auto *behavior = dynamic_cast<BehaviorNode *>(node);
    if (!behavior)
      continue;
    const auto cursor = behavior->QueryMouseCursor(args);
    if (cursor != MouseCursor::Arrow)
      return cursor;
  }
  return MouseCursor::Arrow;
}

EventReply Layout::OnKeyDown(KeyArgs args) {
  if (args.Key == '3') {
    // WasDown 来自 Win32 lParam 第 30 位；忽略自动重复，确保一次按键只切换一次。
    if (!args.WasDown) {
      DebugPanelBorders = !DebugPanelBorders;
      Dirty = true;
      UpdatePanelDebugVisuals();
    }
    return EventReply::Handled;
  }
  if (args.Key != VK_ESCAPE || Dock.Drag.State == PanelDragState::Idle)
    return EventReply::Ignored;
  // Escape 与 capture-lost 共享无副作用取消路径：DockTree 在预览阶段从未
  // 被改动，因此只需复位节点手势和 DragSession 即可恢复原 placement。
  if (CapturedHandler)
    CapturedHandler->CancelPointerCapture();
  CapturedTarget = nullptr;
  CapturedHandler = nullptr;
  Dock.CancelDrag();
  Dirty = true;
  return EventReply::Handled;
}

EventReply Layout::OnMouseDown(MouseMovArgs args) {
  if (args.Button == MouseButton::Left) {
    if (auto *split = Dock.Tree.FindSplitterAt(static_cast<float>(args.X),
                                               static_cast<float>(args.Y))) {
      CapturedSplitter = split->ID;
      return EventReply::Capture;
    }
  }
  auto *target =
      HitAt(static_cast<float>(args.X), static_cast<float>(args.Y));
  if (!target) return EventReply::Ignored;
  if (target->RoutesToScene()) {
    // Scene 内容区通常透传给相机，但外层 SceneNode 的缩放边框与内容相交。
    // 光标命中缩放方向时优先启动 UI 几何手势，否则才把按键留给 3D 场景。
    for (auto *node = target->Parent; node; node = node->Parent) {
      auto *behavior = dynamic_cast<BehaviorNode *>(node);
      if (!behavior || behavior->QueryMouseCursor(args) == MouseCursor::Arrow)
        continue;
      const auto reply = behavior->DispatchMouseDown(args);
      if (reply == EventReply::Capture) {
        CapturedTarget = target;
        CapturedHandler = behavior;
      }
      return reply;
    }
    return EventReply::Ignored;
  }

  CapturedTarget = target;
  // 输入只属于节点和 Behavior；UIObject 是渲染数据，不再接收重复事件。
  for (auto *node = target; node; node = node->Parent) {
    auto *behavior = dynamic_cast<BehaviorNode *>(node);
    if (!behavior)
      continue;
    const auto reply = behavior->DispatchMouseDown(args);
    if (reply == EventReply::Capture)
      CapturedHandler = behavior;
    if (reply != EventReply::Ignored)
      break;
  }
  if (RemoveClosedPanelGroups(*Root)) {
    // 关闭请求在命中分发完全返回后提交，避免按钮处理函数释放自身；重建
    // 索引还会统一清除 Hover/Capture 的观察指针。
    RebuildIndex();
    return EventReply::Handled;
  }
  if (CapturedHandler &&
      dynamic_cast<ResizeBehavior *>(CapturedHandler->CapturedBehavior)) {
    auto *resize = static_cast<ResizeBehavior *>(CapturedHandler->CapturedBehavior);
    const auto *target = resize->GetResizeTarget()
                             ? resize->GetResizeTarget()
                             : CapturedHandler;
    WriteTerminal("[Resize] Started: " + InteractionName(*target));
  }
  if (CapturedHandler &&
      dynamic_cast<DragBehavior *>(CapturedHandler->CapturedBehavior) &&
      CapturedHandler->GetBehavior<DockBehavior>()) {
    Dock.BeginDrag(*CapturedHandler, static_cast<float>(args.X),
                   static_cast<float>(args.Y));
  } else if (auto *tab = dynamic_cast<PanelGroupTabNode *>(CapturedHandler);
             tab && tab->Group && tab->PanelIndex < tab->Group->Panels.size()) {
    // 点击立即激活 Panel；是否构成拖动仍由 DragBehavior 的位移阈值决定。
    tab->Group->ActivatePanel(tab->PanelIndex);
    Dock.BeginPanelDrag(*tab->Group->Panels[tab->PanelIndex], *tab->Group,
                        tab->PanelIndex, static_cast<float>(args.X),
                        static_cast<float>(args.Y));
  }
  // 命中任何 UI 都会阻止事件穿透到场景，即使控件没有主动手势。
  return EventReply::Handled;
}

EventReply Layout::OnMouseMove(MouseMovArgs args) {
  auto *target =
      HitAt(static_cast<float>(args.X), static_cast<float>(args.Y));
  if (!target)
    return EventReply::Ignored;
  if (target->RoutesToScene())
    return EventReply::Ignored;
  for (auto *node = target; node; node = node->Parent)
    if (auto *behavior = dynamic_cast<BehaviorNode *>(node);
        behavior && behavior->DispatchMouseMove(args))
      break;
  return EventReply::Handled;
}

EventReply Layout::OnMouseDrag(MouseMovArgs args) {
  if (CapturedSplitter) {
    Dock.ResizeSplitter(CapturedSplitter, static_cast<float>(args.X),
                        static_cast<float>(args.Y));
    return EventReply::Handled;
  }
  if (!CapturedTarget)
    return EventReply::Ignored;
  auto *drag = CapturedHandler
                   ? dynamic_cast<DragBehavior *>(CapturedHandler->CapturedBehavior)
                   : nullptr;
  auto *resize = CapturedHandler
                     ? dynamic_cast<ResizeBehavior *>(
                           CapturedHandler->CapturedBehavior)
                     : nullptr;
  const bool minimumBlocked =
      resize && IsMinimumBlocked(*resize, *CapturedHandler, args);
  const bool wasMoved = drag && drag->HasGestureMoved();
  if (CapturedHandler)
    CapturedHandler->DispatchMouseDrag(args);
  if (drag && (CapturedHandler->GetBehavior<DockBehavior>() ||
               Dock.Drag.PayloadType == DragPayloadType::Panel)) {
    Dock.UpdateDrag(static_cast<float>(args.X), static_cast<float>(args.Y));
    // 预览层只在拖拽期间进入渲染批次，状态切换必须触发重建。
    Dirty = true;
  }
  if (drag && drag->HasGestureMoved()) {
    const std::string key = InteractionName(*CapturedHandler);
    if (!wasMoved)
      WriteTerminal("[Drag] Started: " + key);
    WriteTerminal("[Drag] Moved: " + key + " (" +
                  std::to_string(args.X) + ", " + std::to_string(args.Y) +
                  ")");
  }
  if (resize) {
    const auto *resizeTarget = resize->GetResizeTarget()
                                   ? resize->GetResizeTarget()
                                   : CapturedHandler;
    const std::string key = InteractionName(*resizeTarget);
    if (minimumBlocked)
      WriteTerminal("[Resize] Blocked at minimum: " + key);
    else
      WriteTerminal("[Resize] Moved: " + key + " (" +
                    std::to_string(static_cast<int>(resizeTarget->Width)) +
                    " x " +
                    std::to_string(static_cast<int>(resizeTarget->Height)) +
                    ")");
  }
  return EventReply::Handled;
}

EventReply Layout::OnMouseUp(MouseMovArgs args) {
  if (CapturedSplitter) {
    Dock.ResizeSplitter(CapturedSplitter, static_cast<float>(args.X),
                        static_cast<float>(args.Y));
    CapturedSplitter = 0;
    return EventReply::Handled;
  }
  if (!CapturedTarget)
    return EventReply::Ignored;
  auto *drag = CapturedHandler
                   ? dynamic_cast<DragBehavior *>(CapturedHandler->CapturedBehavior)
                   : nullptr;
  auto *resize = CapturedHandler
                     ? dynamic_cast<ResizeBehavior *>(
                           CapturedHandler->CapturedBehavior)
                     : nullptr;
  const bool completedDrag = drag && drag->HasGestureMoved();
  const std::string key = CapturedHandler ? InteractionName(*CapturedHandler)
                                          : "";
  const std::string resizeKey =
      resize && resize->GetResizeTarget()
          ? InteractionName(*resize->GetResizeTarget())
          : key;
  if (CapturedHandler)
    CapturedHandler->DispatchMouseUp(args);
  if (drag && (CapturedHandler->GetBehavior<DockBehavior>() ||
               Dock.Drag.PayloadType == DragPayloadType::Panel)) {
    auto *draggedNode = CapturedHandler;
    if (Dock.Drag.PayloadType == DragPayloadType::Panel) {
      CommitPanelDrop(static_cast<float>(args.X), static_cast<float>(args.Y));
    } else {
      Dock.CommitDrag(static_cast<float>(args.X), static_cast<float>(args.Y));
      if (const auto *state = Dock.GetState(*draggedNode);
          state && state->Placement == PanelPlacement::Floating)
        BringToFront(draggedNode);
    }
    Dirty = true;
  }
  if (completedDrag)
    WriteTerminal("[Drag] Completed: " + key);
  if (resize)
    WriteTerminal("[Resize] Completed: " + resizeKey);
  CapturedTarget = nullptr;
  CapturedHandler = nullptr;
  CapturedSplitter = 0;
  return EventReply::Handled;
}

EventReply Layout::OnMouseWheel(MouseWheelArgs args) {
  auto *target =
      HitAt(static_cast<float>(args.X), static_cast<float>(args.Y));
  if (!target)
    return EventReply::Ignored;
  if (target->RoutesToScene())
    return EventReply::Ignored;
  for (auto *node = target; node; node = node->Parent)
    if (auto *behavior = dynamic_cast<BehaviorNode *>(node);
        behavior && behavior->DispatchMouseWheel(args))
      return EventReply::Handled;
  // 与其他指针事件一致，命中 UI 后不允许滚轮穿透到 3D 场景。
  return EventReply::Handled;
}

void Layout::OnPointerCaptureLost() {
  // 窗口失焦或上层路由取消手势时，节点与 Behavior 必须同步释放捕获，避免
  // 下一次按下继承已经失效的拖拽/缩放状态。
  if (CapturedHandler)
    CapturedHandler->CancelPointerCapture();
  CapturedTarget = nullptr;
  CapturedHandler = nullptr;
  Dock.CancelDrag();
  CapturedSplitter = 0;
  Dirty = true;
}

DockRect Layout::GetDockPreview() const {
  if (Dock.Drag.State != PanelDragState::Dragging)
    return {};
  return Dock.Drag.DockTarget &&
                 Dock.Drag.Side != DockSide::Center
             ? Dock.Drag.DockPreviewRect
         : Dock.Drag.PayloadType == DragPayloadType::Panel &&
                   Dock.Drag.TargetGroup
             ? Dock.Drag.DockPreviewRect
             : Dock.Drag.FloatingPreviewRect;
}

void Layout::BringToFront(BaseNode *node) {
  if (!node || !node->Parent)
    return;
  auto &siblings = node->Parent->Children;
  const auto iterator = std::find_if(
      siblings.begin(), siblings.end(),
      [node](const auto &candidate) { return candidate.get() == node; });
  if (iterator == siblings.end() || std::next(iterator) == siblings.end())
    return;
  // 画家顺序由子节点顺序决定；Floating 窗口移到末尾即可同时
  // 获得最上层绘制和逆序命中优先级，无需再引入第二套 z-order。
  auto owner = std::move(*iterator);
  siblings.erase(iterator);
  siblings.push_back(std::move(owner));
  RebuildIndex();
}

bool Layout::CommitPanelDrop(float clientX, float clientY) {
  Dock.UpdateDrag(clientX, clientY);
  const auto drag = Dock.Drag;
  if (drag.State != PanelDragState::Dragging || !drag.PayloadPanel ||
      !drag.SourceGroup || drag.SourceTabIndex < 0) {
    Dock.CancelDrag();
    return false;
  }

  // TargetGroup 只由 UpdateDrag 在空白标题栏或同组 Tab 上设置。不能根据
  // Center Leaf 反查 Group，否则普通内容区 Center 会被错误解释成合并页签。
  auto *targetGroup = drag.TargetGroup;
  if (targetGroup && drag.Side == DockSide::Center) {
    if (targetGroup == drag.SourceGroup) {
      // 同组标题栏空白处不改变顺序；命中另一个 Tab 时交换完整的 Panel/Tab
      // 所有权槽位，拖拽期间仍不触碰真实结构。
      const bool reordered =
          drag.TargetTabIndex < 0 ||
          targetGroup->SwapPanels(
              static_cast<size_t>(drag.SourceTabIndex),
              static_cast<size_t>(drag.TargetTabIndex));
      Dock.CancelDrag();
      if (drag.TargetTabIndex >= 0)
        RebuildIndex();
      return reordered;
    }
    auto panel = drag.SourceGroup->RemovePanel(
        static_cast<size_t>(drag.SourceTabIndex));
    if (!panel) {
      Dock.CancelDrag();
      return false;
    }
    targetGroup->AddPanel(std::move(panel));
    targetGroup->ActivatePanel(targetGroup->Panels.size() - 1);
    if (drag.SourceGroup->Panels.empty())
      Dock.Remove(*drag.SourceGroup);
    Dock.CancelDrag();
    RebuildIndex();
    return true;
  }

  // 唯一 Panel 投向自己的边缘没有可保留的目标 Leaf；保持原结构比先删除
  // 来源再尝试命中失效 ID 更确定。多 Panel 来源仍可正常拆出新 Group。
  if (drag.Side != DockSide::Center &&
      drag.DockTarget == drag.SourceNode &&
      drag.SourceGroup->Panels.size() == 1) {
    Dock.CancelDrag();
    return true;
  }

  auto panel = drag.SourceGroup->RemovePanel(
      static_cast<size_t>(drag.SourceTabIndex));
  if (!panel) {
    Dock.CancelDrag();
    return false;
  }
  if (drag.SourceGroup->Panels.empty())
    Dock.Remove(*drag.SourceGroup);

  auto group = std::make_unique<PanelGroupNode>();
  auto *newGroup = group.get();
  newGroup->Key = panel->Key + ".__group";
  newGroup->Style.Width = drag.FloatingPreviewRect.Width;
  newGroup->Style.Height = drag.FloatingPreviewRect.Height;
  newGroup->AddPanel(std::move(panel));
  Root->BaseNode::AddChild(std::move(group));
  const bool placed = Dock.PlaceNew(*newGroup, drag.DockTarget, drag.Side,
                                    drag.FloatingPreviewRect);
  Dock.CancelDrag();
  RebuildIndex();
  return placed;
}

void Layout::NormalizePanelGroups(BaseNode &parent) {
  // PanelGroup Pages 是显式的 Panel 所有权边界，其内部不能再次包装。
  if (parent.Key == "__pages" &&
      dynamic_cast<PanelGroupNode *>(parent.Parent))
    return;
  for (auto &child : parent.Children) {
    if (auto *panel = dynamic_cast<PanelNode *>(child.get())) {
      const auto *dock = panel->GetBehavior<DockBehavior>();
      if (!dock || !dock->Properties.Enabled)
        continue;
      auto panelOwner = std::unique_ptr<PanelNode>(
          static_cast<PanelNode *>(child.release()));
      auto group = std::make_unique<PanelGroupNode>();
      group->Key = panelOwner->Key + ".__group";
      group->Style = panelOwner->Style;
      group->Style.Padding = 0.0f;
      group->GetBehavior<DockBehavior>()->Properties = dock->Properties;
      group->AddPanel(std::move(panelOwner));
      group->Parent = &parent;
      child = std::move(group);
      continue;
    }
    NormalizePanelGroups(*child);
  }
}

bool Layout::RemoveClosedPanelGroups(BaseNode &parent) {
  bool removed = false;
  for (auto iterator = parent.Children.begin();
       iterator != parent.Children.end();) {
    auto *group = dynamic_cast<PanelGroupNode *>(iterator->get());
    if (group && (group->CloseRequested || group->Panels.empty())) {
      // 先撤销事务状态再释放视觉树所有权，保证 DockTree、捕获和拖拽会话
      // 不会留下指向已析构 Group 的观察指针。
      Dock.Remove(*group);
      iterator = parent.Children.erase(iterator);
      removed = true;
      continue;
    }
    removed = RemoveClosedPanelGroups(**iterator) || removed;
    ++iterator;
  }
  return removed;
}

BaseNode *Layout::Find(const std::string &key) const {
  for (auto *node : Nodes) {
    if (node->Key != key)
      continue;
    // 默认单页 Group 对外保留 Panel 的查找身份；声明协调仍以
    // Group 内部 Key 复用容器，业务代码不需要因自动包装而改变类型。
    if (auto *group = dynamic_cast<PanelGroupNode *>(node);
        group && group->Panels.size() == 1)
      return group->Panels.front();
      return node;
  }
  return nullptr;
}

bool Layout::ConsumeDirty() {
  const bool result = Dirty;
  Dirty = false;
  return result;
}

std::vector<GameObject *> Layout::GetUO() const {
  std::vector<GameObject *> result;
  result.reserve(Visuals.size() + PanelDebugVisuals.size() + 1);
  for (auto *v : Visuals) {
    assert (v->UO);
    result.push_back(v->UO.get());
  }
  // 预览不是 UI 树的常驻节点；仅在有效拖拽帧追加，保持普通
  // 布局的可渲染对象集与引入 Dock 之前一致。
  if (DockPreviewVisual && Dock.Drag.State == PanelDragState::Dragging) {
    assert(DockPreviewVisual->UO);
    result.push_back(DockPreviewVisual->UO.get());
  }
  for (const auto &visual : PanelDebugVisuals) {
    assert(visual && visual->UO);
    result.push_back(visual->UO.get());
  }
  return result;
}

std::vector<GameObject *> Layout::GetMainUO() const {
  std::vector<GameObject *> result;
  for (auto *visual : Visuals) {
    const BaseNode *rootChild = visual;
    while (rootChild->Parent && rootChild->Parent != Root.get())
      rootChild = rootChild->Parent;
    if (!Dock.IsFloating(*rootChild))
      result.push_back(visual->UO.get());
  }
  if (DockPreviewVisual && Dock.Drag.State == PanelDragState::Dragging)
    result.push_back(DockPreviewVisual->UO.get());
  for (const auto &visual : PanelDebugVisuals)
    result.push_back(visual->UO.get());
  return result;
}

std::vector<GameObject *> Layout::GetSubtreeUO(const BaseNode &root) const {
  std::vector<GameObject *> result;
  for (auto *visual : Visuals) {
    for (const BaseNode *node = visual; node; node = node->Parent) {
      if (node != &root)
        continue;
      result.push_back(visual->UO.get());
      break;
    }
  }
  return result;
}

std::vector<TextNode *> Layout::GetMainTexts() const {
  std::vector<TextNode *> result;
  for (auto *text : Texts) {
    const BaseNode *rootChild = text;
    while (rootChild->Parent && rootChild->Parent != Root.get())
      rootChild = rootChild->Parent;
    if (!Dock.IsFloating(*rootChild))
      result.push_back(text);
  }
  return result;
}

std::vector<TextNode *> Layout::GetSubtreeTexts(const BaseNode &root) const {
  std::vector<TextNode *> result;
  for (auto *text : Texts) {
    for (const BaseNode *node = text; node; node = node->Parent) {
      if (node != &root)
        continue;
      result.push_back(text);
      break;
    }
  }
  return result;
}

SceneNode *Layout::GetSceneNode() const {
  for (auto *scene : Scenes)
    if (scene->Visible)
      return scene;
  return nullptr;
}

void Layout::WriteTerminal(const std::string &message) const {
  for (auto *terminal : Terminals)
    if (terminal && terminal->Visible)
      terminal->AppendMessage(message);
}

// 每帧计算布局的入口函数
void Layout::Calculate(float w, float h) {
  Dock.ApplyLayout(w, h);
  LayoutEngine::Calculate(*Root, w, h);
  const DirectX::XMFLOAT4 clip{0.0f, 0.0f, w, h};
  UpdateTree(*Root, 0, 0, clip, true);
  UpdateDockPreview();
  UpdatePanelDebugVisuals();

  bool scrollChanged = false;
  for (auto *terminal : Terminals)
    if (terminal && terminal->Visible)
      scrollChanged = terminal->ApplyPendingScroll() || scrollChanged;
  if (scrollChanged) {
    // 滚动范围只能在布局完成后确定；偏移变化不需要再次测量尺寸，但必须在
    // 绘制前重新传播绝对坐标，否则最新日志仍使用上一帧位置并被 viewport 裁掉。
    UpdateTree(*Root, 0, 0, clip, false);
  }
}

void Layout::UpdateTree(BaseNode &node, float parentX, float parentY,
                        const DirectX::XMFLOAT4 &clip,
                        bool dispatchAfterLayout, bool parentVisible) {
  // 计算框位于父内容坐标系；屏幕空间偏移只在这里累加一次，避免求解器依赖渲染坐标。
  const float x = node.Computed.Left;
  const float y = node.Computed.Top;
  const float width = node.Computed.Width;
  const float height = node.Computed.Height;

  // 2. 然后计算绝对位置
  float absX = parentX + x;
  float absY = parentY + y;

  // 3. 记录本次数据，并更新
  node.Left = absX;
  node.Top = absY;
  node.Width = width;
  node.Height = height;
  node.VisibleClip = clip;
  node.EffectiveVisible = parentVisible && node.Visible;
  node.Synchronize();

  // 4. 计算裁剪矩形
  DirectX::XMFLOAT4 childClip = clip;
  if (node.ClipChildren) {
    childClip.x = (std::max)(childClip.x, absX);
    childClip.y = (std::max)(childClip.y, absY);
    childClip.z = (std::min)(childClip.z, absX + width);
    childClip.w = (std::min)(childClip.w, absY + height);
  }

  // 5. 迭代子节点
  for (const auto &child : node.Children) {
    DirectX::XMFLOAT4 resolvedClip = childClip;
    if (&node == Root.get() && Dock.IsFloating(*child)) {
      // Floating 子树由独立 HWND 呈现，不能继承主客户区裁剪；仍保存 Layout
      // 全局坐标，使跨窗口拖回 DockTree 时无需切换坐标空间或复制节点状态。
      resolvedClip = {child->Computed.Left, child->Computed.Top,
                      child->Computed.Left + child->Computed.Width,
                      child->Computed.Top + child->Computed.Height};
    }
    UpdateTree(*child, absX + node.ChildOffsetX, absY + node.ChildOffsetY,
               resolvedClip, dispatchAfterLayout, node.EffectiveVisible);
  }

  // 6. 事件通知
  // 非活动页仍保留节点状态，但不应运行 Panel 内容的逐帧更新；可见性与
  // 输入命中使用同一传播结果，避免隐藏页在后台继续产生 UI 状态变化。
  if (dispatchAfterLayout && node.EffectiveVisible)
    node.DispatchAfterLayout();
}

void Layout::UpdateDockPreview() {
  if (!DockPreviewVisual)
    return;
  const auto rect = GetDockPreview();
  DockPreviewVisual->Visible = Dock.Drag.State == PanelDragState::Dragging;
  DockPreviewVisual->EffectiveVisible = DockPreviewVisual->Visible;
  DockPreviewVisual->Left = rect.Left;
  DockPreviewVisual->Top = rect.Top;
  DockPreviewVisual->Width = rect.Width;
  DockPreviewVisual->Height = rect.Height;
  DockPreviewVisual->VisibleClip = {0.0f, 0.0f, Root->Width, Root->Height};
  DockPreviewVisual->Synchronize();
}

void Layout::UpdatePanelDebugVisuals() {
  if (!DebugPanelBorders) {
    if (!PanelDebugVisuals.empty()) {
      PanelDebugVisuals.clear();
      Dirty = true;
    }
    return;
  }
  std::vector<const BaseNode *> bounds;
  for (auto *node : Nodes) {
    if (!node->EffectiveVisible || node->Width <= 0.0f ||
        node->Height <= 0.0f)
      continue;
    bool belongsToPanelTree = false;
    for (auto *ancestor = node; ancestor; ancestor = ancestor->Parent) {
      if (dynamic_cast<PanelNode *>(ancestor) ||
          dynamic_cast<PanelGroupNode *>(ancestor)) {
        belongsToPanelTree = true;
        break;
      }
    }
    if (belongsToPanelTree)
      bounds.push_back(node);
  }
  if (PanelDebugVisuals.size() != bounds.size()) {
    PanelDebugVisuals.clear();
    PanelDebugVisuals.reserve(bounds.size());
    for (size_t index = 0; index < bounds.size(); ++index) {
      auto visual = std::make_unique<RectNode>();
      visual->Key = "__debug_panel_border";
      visual->HitTestVisible = false;
      visual->SetColor({0.0f, 0.0f, 0.0f, 0.0f});
      visual->SetBorder({1.0f, 0.0f, 0.0f, 1.0f}, 2.0f);
      visual->SetCornerRadius(0.0f);
      PanelDebugVisuals.push_back(std::move(visual));
    }
    // 渲染批次缓存 UIObject 指针；只有数量变化才需要重建，普通布局帧复用对象。
    Dirty = true;
  }
  for (size_t index = 0; index < bounds.size(); ++index) {
    auto &visual = PanelDebugVisuals[index];
    visual->Left = bounds[index]->Left;
    visual->Top = bounds[index]->Top;
    visual->Width = bounds[index]->Width;
    visual->Height = bounds[index]->Height;
    // 与目标节点使用同一裁剪矩形，滚动内容的诊断线不会穿出 viewport；
    // Text/Image 等非矩形布局节点仍通过独立 Rect 覆盖显示自己的布局框。
    visual->VisibleClip = bounds[index]->VisibleClip;
    visual->Visible = true;
    visual->EffectiveVisible = true;
    visual->Synchronize();
  }
}
