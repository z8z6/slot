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
      CapturedHandler->GetBehavior<DockBehavior>())
    Dock.BeginDrag(*CapturedHandler, static_cast<float>(args.X),
                   static_cast<float>(args.Y));
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
  if (drag && CapturedHandler->GetBehavior<DockBehavior>()) {
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
  if (drag && CapturedHandler->GetBehavior<DockBehavior>()) {
    auto *draggedNode = CapturedHandler;
    Dock.CommitDrag(static_cast<float>(args.X), static_cast<float>(args.Y));
    if (const auto *state = Dock.GetState(*draggedNode);
        state && state->Placement == PanelPlacement::Floating)
      BringToFront(draggedNode);
    CommitPanelGroupMerge();
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
                 (Dock.Drag.Side != DockSide::Center ||
                  Dock.Drag.TargetGroupTitle)
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

void Layout::CommitPanelGroupMerge() {
  auto *source = Dock.CommittedMergeSource;
  auto *target = Dock.CommittedMergeTarget;
  Dock.CommittedMergeSource = nullptr;
  Dock.CommittedMergeTarget = nullptr;
  if (!source || !target || !source->Parent)
    return;
  auto *parent = source->Parent;
  target->MergeFrom(*source);
  auto &siblings = parent->Children;
  const auto iterator = std::find_if(
      siblings.begin(), siblings.end(),
      [source](const auto &candidate) { return candidate.get() == source; });
  if (iterator != siblings.end())
    siblings.erase(iterator);
  RebuildIndex();
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
  result.reserve(Visuals.size() + 1);
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
    UpdateTree(*child, absX + node.ChildOffsetX, absY + node.ChildOffsetY,
               childClip, dispatchAfterLayout, node.EffectiveVisible);
  }

  // 6. 事件通知
  if (dispatchAfterLayout)
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
