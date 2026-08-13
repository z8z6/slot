#include "UI/Dock/DockWorkspace.h"

#include "UI/Behavior/DockBehavior.h"
#include "UI/Layout/BaseNode.h"
#include "UI/Layout/BehaviorNode.h"
#include "UI/Layout/PanelGroupNode.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <unordered_set>

using namespace z8::ui;

namespace {

DockBehavior *GetDock(BaseNode *node) {
  auto *behaviorNode = dynamic_cast<BehaviorNode *>(node);
  return behaviorNode ? behaviorNode->GetBehavior<DockBehavior>() : nullptr;
}

DockSide PlacementSide(DockPlacement placement) {
  switch (placement) {
  case DockPlacement::Left: return DockSide::Left;
  case DockPlacement::Right: return DockSide::Right;
  case DockPlacement::Top: return DockSide::Top;
  case DockPlacement::Bottom: return DockSide::Bottom;
  default: return DockSide::Center;
  }
}

} // namespace

void DockWorkspace::BuildInitialTree(const std::vector<BaseNode *> &panels,
                                     float width, float height) {
  Tree.Clear();
  BaseNode *fill = nullptr;
  for (auto *panel : panels) {
    const auto *dock = GetDock(panel);
    if (dock && dock->Properties.Placement == DockPlacement::Fill) {
      fill = panel;
      break;
    }
  }
  if (!fill && !panels.empty())
    fill = panels.back();
  if (!fill)
    return;
  auto *root = Tree.AddPanel(fill);
  States[fill] = {PanelPlacement::Docked, root->ID, {}};

  for (auto *panel : panels) {
    if (panel == fill)
      continue;
    const auto *dock = GetDock(panel);
    DockTransaction transaction;
    transaction.Panel = panel;
    transaction.TargetNode = Tree.FindPanelLeaf(fill)->ID;
    transaction.TargetSide = dock ? PlacementSide(dock->Properties.Placement)
                                  : DockSide::Left;
    if (transaction.TargetSide == DockSide::Center)
      transaction.TargetSide = DockSide::Left;
    Tree.Layout({0.0f, 0.0f, width, height});
    const auto targetRect = Tree.Find(transaction.TargetNode)->Rect;
    const float axisExtent = transaction.TargetSide == DockSide::Left ||
                                     transaction.TargetSide == DockSide::Right
                                 ? targetRect.Width
                                 : targetRect.Height;
    const bool explicitExtent = dock &&
                                dock->Properties.Placement != DockPlacement::Auto &&
                                dock->Properties.Placement != DockPlacement::Fill;
    const float requested = explicitExtent ? dock->Properties.Extent
                                           : axisExtent * 0.5f;
    const float newRatio = axisExtent > 0.0f
                               ? std::clamp(requested / axisExtent, 0.05f, 0.95f)
                               : 0.5f;
    transaction.SplitRatio = transaction.TargetSide == DockSide::Right ||
                                     transaction.TargetSide == DockSide::Bottom
                                 ? 1.0f - newRatio
                                 : newRatio;
    Tree.Commit(transaction);
    States[panel] = {PanelPlacement::Docked,
                     Tree.FindPanelLeaf(panel)->ID, {}};
  }
}

void DockWorkspace::Reconcile(const std::vector<BaseNode *> &nodes) {
  std::vector<BaseNode *> panels;
  std::unordered_set<BaseNode *> alive;
  for (auto *node : nodes) {
    auto *dock = GetDock(node);
    if (dock && dock->Properties.Enabled) {
      panels.push_back(node);
      alive.insert(node);
    }
  }
  for (auto iterator = States.begin(); iterator != States.end();) {
    if (!alive.contains(iterator->first)) {
      Tree.RemovePanel(iterator->first);
      iterator->first->LayoutManaged = false;
      iterator = States.erase(iterator);
    } else {
      ++iterator;
    }
  }
  if (States.empty()) {
    PendingInitialPanels = panels;
    return;
  }
  for (auto *panel : panels) {
    if (States.contains(panel))
      continue;
    DockNode *target = Tree.Root.get();
    while (target && target->Type == DockNodeType::Split)
      target = target->ChildB.get();
    if (!target) {
      target = Tree.AddPanel(panel);
    } else {
      // 运行时新 Panel 不能通过 Center 暗中叠放成页签；多页组由
      // PanelGroupNode 显式表达。新节点默认在工作区右侧建立 Split。
      Tree.Commit({panel, 0, target->ID, DockSide::Right, false, {}, 0.5f});
    }
    States[panel] = {PanelPlacement::Docked,
                     Tree.FindPanelLeaf(panel)->ID, {}};
  }
}

void DockWorkspace::ApplyPanelRect(BaseNode &panel, const DockRect &rect,
                                   bool visible) {
  panel.Visible = visible;
  panel.Style.Margin = 0.0f;
  panel.Style.Position = PositionType::Absolute;
  panel.Style.Left = rect.Left;
  panel.Style.Top = rect.Top;
  panel.Style.Width = (std::max)(0.0f, rect.Width);
  panel.Style.Height = (std::max)(0.0f, rect.Height);
  panel.Style.FlexGrow = 0.0f;
  panel.Style.FlexShrink = 0.0f;
}

void DockWorkspace::ApplyLayout(float width, float height) {
  if (!PendingInitialPanels.empty()) {
    BuildInitialTree(PendingInitialPanels, width, height);
    PendingInitialPanels.clear();
  }
  // DockEnabled 可在运行时改变。禁用后节点回到普通布局/交互路径，不能继续被
  // 旧 DockState 覆盖；这也让独立控件测试无需重建整棵 UI 树。
  for (auto iterator = States.begin(); iterator != States.end();) {
    const auto *dock = GetDock(iterator->first);
    if (!dock || !dock->Properties.Enabled) {
      Tree.RemovePanel(iterator->first);
      iterator->first->LayoutManaged = false;
      iterator = States.erase(iterator);
    } else {
      ++iterator;
    }
  }
  Tree.Layout({0.0f, 0.0f, (std::max)(0.0f, width),
               (std::max)(0.0f, height)});
  for (auto &[panel, state] : States) {
    if (state.Placement == PanelPlacement::Floating) {
      panel->LayoutManaged = false;
      ApplyPanelRect(*panel, state.FloatingRect, true);
      continue;
    }
    auto *leaf = Tree.FindPanelLeaf(panel);
    if (!leaf)
      continue;
    state.DockNode = leaf->ID;
    panel->LayoutManaged = true;
    ApplyPanelRect(*panel, leaf->Rect, true);
  }
}

void DockWorkspace::BeginDrag(BaseNode &panel, float clientX, float clientY) {
  const auto stateIterator = States.find(&panel);
  if (stateIterator == States.end())
    return;
  Drag = {};
  Drag.State = PanelDragState::Pressed;
  Drag.PayloadType = DragPayloadType::PanelGroup;
  Drag.PayloadGroup = dynamic_cast<PanelGroupNode *>(&panel);
  Drag.SourceGroup = Drag.PayloadGroup;
  Drag.Panel = &panel;
  Drag.SourceWasFloating =
      stateIterator->second.Placement == PanelPlacement::Floating;
  Drag.SourceFloatingRect = stateIterator->second.FloatingRect;
  if (auto *leaf = Tree.FindPanelLeaf(&panel)) {
    Drag.SourceNode = leaf->ID;
    const auto iterator = std::find(leaf->Panels.begin(), leaf->Panels.end(), &panel);
    Drag.SourceTabIndex = static_cast<int>(
        std::distance(leaf->Panels.begin(), iterator));
  }
  Drag.PressClientX = clientX;
  Drag.PressClientY = clientY;
  Drag.MouseClientX = clientX;
  Drag.MouseClientY = clientY;
  Drag.GrabOffsetX = clientX - panel.Left;
  Drag.GrabOffsetY = clientY - panel.Top;
  Drag.FloatingPreviewRect = {panel.Left, panel.Top, panel.Width, panel.Height};
}

void DockWorkspace::BeginPanelDrag(PanelNode &panel,
                                   PanelGroupNode &sourceGroup,
                                   size_t sourceTabIndex, float clientX,
                                   float clientY) {
  const auto stateIterator = States.find(&sourceGroup);
  if (stateIterator == States.end())
    return;
  Drag = {};
  Drag.State = PanelDragState::Pressed;
  Drag.PayloadType = DragPayloadType::Panel;
  Drag.PayloadPanel = &panel;
  Drag.SourceGroup = &sourceGroup;
  Drag.Panel = &sourceGroup;
  Drag.SourceTabIndex = static_cast<int>(sourceTabIndex);
  Drag.SourceWasFloating =
      stateIterator->second.Placement == PanelPlacement::Floating;
  Drag.SourceFloatingRect = stateIterator->second.FloatingRect;
  if (auto *leaf = Tree.FindPanelLeaf(&sourceGroup))
    Drag.SourceNode = leaf->ID;
  Drag.PressClientX = clientX;
  Drag.PressClientY = clientY;
  Drag.MouseClientX = clientX;
  Drag.MouseClientY = clientY;
  Drag.GrabOffsetX = clientX - sourceGroup.Left;
  Drag.GrabOffsetY = clientY - sourceGroup.Top;
  // Panel 预览沿用来源 Group 的外框，但不创建临时节点；真实单页 Group
  // 只允许在 MouseUp Commit 时进入所有权树。
  Drag.FloatingPreviewRect = {sourceGroup.Left, sourceGroup.Top,
                              sourceGroup.Width, sourceGroup.Height};
}

DockSide DockWorkspace::DetectSide(const DockRect &rect, float clientX,
                                   float clientY) const {
  const float nx = rect.Width > 0.0f ? (clientX - rect.Left) / rect.Width : 0.5f;
  const float ny = rect.Height > 0.0f ? (clientY - rect.Top) / rect.Height : 0.5f;
  if (nx < 0.25f) return DockSide::Left;
  if (nx > 0.75f) return DockSide::Right;
  if (ny < 0.25f) return DockSide::Top;
  if (ny > 0.75f) return DockSide::Bottom;
  return DockSide::Center;
}

void DockWorkspace::UpdateDrag(float clientX, float clientY) {
  if (!Drag.Panel || Drag.State == PanelDragState::Idle)
    return;
  Drag.MouseClientX = clientX;
  Drag.MouseClientY = clientY;
  const float dx = clientX - Drag.PressClientX;
  const float dy = clientY - Drag.PressClientY;
  constexpr float threshold = 5.0f;
  if (Drag.State == PanelDragState::Pressed &&
      std::sqrt(dx * dx + dy * dy) <= threshold)
    return;
  Drag.State = PanelDragState::Dragging;
  Drag.FloatingPreviewRect.Left = clientX - Drag.GrabOffsetX;
  Drag.FloatingPreviewRect.Top = clientY - Drag.GrabOffsetY;
  auto *target = Tree.FindLeafAt(clientX, clientY);
  Drag.DockTarget = target ? target->ID : 0;
  Drag.TargetTabIndex = -1;
  Drag.TargetGroup = nullptr;
  if (target) {
    auto *targetGroup = target->Panels.size() == 1
                            ? dynamic_cast<PanelGroupNode *>(
                                  target->Panels.front())
                            : nullptr;
    int targetTabIndex = -1;
    const bool hitsGroupHeader =
        Drag.PayloadType == DragPayloadType::Panel && targetGroup &&
        targetGroup->HeaderNode &&
        targetGroup->HeaderNode->Contains(clientX, clientY);
    if (hitsGroupHeader) {
      for (size_t index = 0; index < targetGroup->Tabs.size(); ++index) {
        if (!targetGroup->Tabs[index]->Contains(clientX, clientY))
          continue;
        targetTabIndex = static_cast<int>(index);
        break;
      }
    }
    // 只有 Tab 与关闭按钮之间的 DragHandle 才是“加入 Group”的明确落点。
    // 已有 Tab 仅允许在来源 Group 内交换；跨 Group 的已有 Tab 和内容区
    // Center 都保持 Center 语义，并在 Commit 时形成独立 Floating Group。
    const bool hitsEmptyHeader =
        hitsGroupHeader && targetGroup->DragHandleNode &&
        targetGroup->DragHandleNode->Contains(clientX, clientY);
    const bool reordersSourceTab =
        hitsGroupHeader && targetGroup == Drag.SourceGroup &&
        targetTabIndex >= 0;
    if (hitsEmptyHeader || reordersSourceTab) {
      Drag.TargetGroup = targetGroup;
      Drag.TargetTabIndex = targetTabIndex;
    }
    Drag.Side = hitsGroupHeader
                    ? DockSide::Center
                    : DetectSide(target->Rect, clientX, clientY);
    Drag.DockPreviewRect = Tree.GetPreviewRect(*target, Drag.Side);
  } else {
    Drag.DockPreviewRect = {};
    // Floating Group 不属于 DockTree，但自己的 Tab 仍必须支持同组重排。
    // 跨浮动窗口的 z-order 命中留给窗口层；这里仅补足当前 source 的确定语义。
    auto *source = Drag.PayloadType == DragPayloadType::Panel
                       ? Drag.SourceGroup
                       : nullptr;
    if (source && source->HeaderNode &&
        source->HeaderNode->Contains(clientX, clientY)) {
      Drag.TargetGroup = source;
      Drag.Side = DockSide::Center;
      for (size_t index = 0; index < source->Tabs.size(); ++index) {
        if (!source->Tabs[index]->Contains(clientX, clientY))
          continue;
        Drag.TargetTabIndex = static_cast<int>(index);
        Drag.DockPreviewRect = {source->Tabs[index]->Left,
                                source->Tabs[index]->Top,
                                source->Tabs[index]->Width,
                                source->Tabs[index]->Height};
        break;
      }
    }
  }
}

bool DockWorkspace::CommitDrag(float clientX, float clientY) {
  if (!Drag.Panel || Drag.State == PanelDragState::Idle ||
      Drag.PayloadType != DragPayloadType::PanelGroup)
    return false;
  UpdateDrag(clientX, clientY);
  if (Drag.State != PanelDragState::Dragging) {
    CancelDrag();
    return false;
  }
  DockTransaction transaction;
  transaction.Panel = Drag.Panel;
  transaction.SourceNode = Drag.SourceNode;
  transaction.TargetNode = Drag.DockTarget;
  transaction.TargetSide = Drag.Side;
  // Unity 式 Center 落点表示创建浮动窗口，不与目标 Leaf 重叠。
  // 需要页签时使用 PanelGroupNode，使 Dock 结构与控件组合语义分离。
  transaction.TargetFloating = Drag.DockTarget == 0 || Drag.Side == DockSide::Center;
  transaction.FloatingRect = Drag.FloatingPreviewRect;
  std::ostringstream trace;
  trace << "DockTree BEFORE\n" << Tree.Dump()
        << "Dock Transaction panel="
        << (transaction.Panel->Key.empty() ? transaction.Panel->TypeName()
                                           : transaction.Panel->Key)
        << " source=#" << transaction.SourceNode
        << " target=#" << transaction.TargetNode
        << " side=" << static_cast<int>(transaction.TargetSide)
        << " floating=" << transaction.TargetFloating << '\n';
  const bool committed = transaction.TargetFloating || Tree.Commit(transaction);
  if (committed) {
    auto &state = States[transaction.Panel];
    state.Placement = transaction.TargetFloating ? PanelPlacement::Floating
                                                 : PanelPlacement::Docked;
    state.FloatingRect = transaction.FloatingRect;
    state.DockNode = transaction.TargetFloating
                         ? 0
                         : Tree.FindPanelLeaf(transaction.Panel)->ID;
    if (transaction.TargetFloating)
      Tree.RemovePanel(transaction.Panel);
  }
  trace << "DockTree AFTER\n" << Tree.Dump()
        << "Commit result=" << committed << '\n';
  LastTransactionTrace = trace.str();
  CancelDrag();
  return committed;
}

void DockWorkspace::CancelDrag() { Drag = {}; }

bool DockWorkspace::PlaceNew(BaseNode &group, DockNodeID target,
                             DockSide side,
                             const DockRect &floatingRect) {
  if (States.contains(&group))
    return false;
  const bool floating = target == 0 || side == DockSide::Center;
  if (floating) {
    States[&group] = {PanelPlacement::Floating, 0, floatingRect};
    group.LayoutManaged = false;
    return true;
  }
  if (!Tree.Root) {
    auto *leaf = Tree.AddPanel(&group);
    if (!leaf)
      return false;
  } else if (!Tree.Commit({&group, 0, target, side, false, {}, 0.5f})) {
    return false;
  }
  auto *leaf = Tree.FindPanelLeaf(&group);
  if (!leaf)
    return false;
  States[&group] = {PanelPlacement::Docked, leaf->ID, {}};
  group.LayoutManaged = true;
  return true;
}

bool DockWorkspace::Remove(BaseNode &panel) {
  if (Drag.Panel == &panel)
    CancelDrag();
  const bool removedFromTree = Tree.RemovePanel(&panel);
  const bool removedState = States.erase(&panel) != 0;
  panel.LayoutManaged = false;
  return removedFromTree || removedState;
}

const PanelDockState *DockWorkspace::GetState(const BaseNode &panel) const {
  const auto iterator = States.find(const_cast<BaseNode *>(&panel));
  if (iterator != States.end())
    return &iterator->second;
  const auto *panelNode = dynamic_cast<const PanelNode *>(&panel);
  if (!panelNode || !panelNode->Group)
    return nullptr;
  const auto groupIterator = States.find(panelNode->Group);
  return groupIterator == States.end() ? nullptr : &groupIterator->second;
}

std::vector<BaseNode *> DockWorkspace::GetFloatingPanels() const {
  std::vector<BaseNode *> result;
  result.reserve(States.size());
  for (const auto &[panel, state] : States)
    if (state.Placement == PanelPlacement::Floating)
      result.push_back(panel);
  return result;
}

bool DockWorkspace::IsDocked(const BaseNode &panel) const {
  const auto *state = GetState(panel);
  return state && state->Placement == PanelPlacement::Docked;
}

bool DockWorkspace::IsFloating(const BaseNode &panel) const {
  const auto *state = GetState(panel);
  return state && state->Placement == PanelPlacement::Floating;
}

bool DockWorkspace::UpdateFloatingRect(BaseNode &panel,
                                       const DockRect &rect) {
  auto iterator = States.find(&panel);
  if (iterator == States.end() ||
      iterator->second.Placement != PanelPlacement::Floating)
    return false;
  iterator->second.FloatingRect = rect;
  return true;
}

bool DockWorkspace::ResizeSplitter(DockNodeID split, float clientX,
                                   float clientY) {
  return Tree.ResizeSplitter(split, clientX, clientY);
}

std::string DockWorkspace::DumpDebug() const {
  std::ostringstream stream;
  if (!LastTransactionTrace.empty())
    stream << LastTransactionTrace;
  stream << Tree.Dump();
  stream << "Drag state=" << static_cast<int>(Drag.State)
         << " panel="
         << (Drag.Panel ? (Drag.Panel->Key.empty() ? Drag.Panel->TypeName()
                                                   : Drag.Panel->Key)
                        : "<none>")
         << " source=#" << Drag.SourceNode << " target=#" << Drag.DockTarget
         << " mouseClient=(" << Drag.MouseClientX << ',' << Drag.MouseClientY
         << ") side=" << static_cast<int>(Drag.Side) << '\n';
  return stream.str();
}

bool DockWorkspace::Validate(std::string *error) const {
  if (!Tree.Validate(error))
    return false;
  const auto fail = [error](const std::string &message) {
    if (error)
      *error = message;
    return false;
  };
  for (const auto &[panel, state] : States) {
    const auto *leaf = Tree.FindPanelLeaf(panel);
    if (state.Placement == PanelPlacement::Docked) {
      if (!leaf || leaf->ID != state.DockNode)
        return fail("Docked panel state does not match DockTree ownership.");
    } else if (leaf) {
      return fail("Floating panel must not belong to DockTree.");
    }
  }
  return true;
}
