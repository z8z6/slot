#include "UI/Dock/DockTree.h"

#include "UI/Layout/BaseNode.h"
#include "UI/Layout/PanelGroupNode.h"
#include "UI/Layout/PanelNode.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <unordered_set>

using namespace z8::ui;

namespace {

DockNode *FindNode(DockNode *node, DockNodeID id) {
  if (!node || node->ID == id)
    return node;
  if (auto *result = FindNode(node->ChildA.get(), id))
    return result;
  return FindNode(node->ChildB.get(), id);
}

DockNode *FindLeafAtNode(DockNode *node, float x, float y) {
  if (!node || !node->Rect.Contains(x, y))
    return nullptr;
  if (node->Type == DockNodeType::Leaf)
    return node;
  if (auto *result = FindLeafAtNode(node->ChildB.get(), x, y))
    return result;
  return FindLeafAtNode(node->ChildA.get(), x, y);
}

DockNode *FindPanel(DockNode *node, const BaseNode *panel) {
  if (!node)
    return nullptr;
  if (node->Type == DockNodeType::Leaf &&
      std::find(node->Panels.begin(), node->Panels.end(), panel) !=
          node->Panels.end())
    return node;
  if (auto *result = FindPanel(node->ChildA.get(), panel))
    return result;
  return FindPanel(node->ChildB.get(), panel);
}

DockNode *FindSplitter(DockNode *node, float x, float y, float tolerance) {
  if (!node || node->Type != DockNodeType::Split ||
      !node->Rect.Contains(x, y))
    return nullptr;
  const float boundary = node->Axis == SplitAxis::Vertical
                             ? node->ChildA->Rect.Left + node->ChildA->Rect.Width
                             : node->ChildA->Rect.Top + node->ChildA->Rect.Height;
  const float coordinate = node->Axis == SplitAxis::Vertical ? x : y;
  if (node->SplitterResizable &&
      std::abs(coordinate - boundary) <= tolerance)
    return node;
  if (auto *result = FindSplitter(node->ChildB.get(), x, y, tolerance))
    return result;
  return FindSplitter(node->ChildA.get(), x, y, tolerance);
}

void DumpNode(const DockNode &node, std::ostringstream &stream,
              const std::string &indent) {
  if (node.Type == DockNodeType::Leaf) {
    stream << indent << "Leaf #" << node.ID << " panels=" << node.Panels.size()
           << '\n';
    for (const auto *panel : node.Panels)
      stream << indent << "  - "
             << (panel->Key.empty() ? panel->TypeName() : panel->Key) << '\n';
    return;
  }
  stream << indent << "Split #" << node.ID << ' '
         << (node.Axis == SplitAxis::Vertical ? "Vertical" : "Horizontal")
         << " ratio=" << node.SplitRatio << '\n';
  DumpNode(*node.ChildA, stream, indent + "  A ");
  DumpNode(*node.ChildB, stream, indent + "  B ");
}

float MinimumExtent(const DockNode &node, SplitAxis axis) {
  if (node.Type == DockNodeType::Leaf) {
    float result = 0.0f;
    for (const auto *panel : node.Panels)
      result = (std::max)(result, axis == SplitAxis::Vertical
                                     ? panel->Style.MinWidth
                                     : panel->Style.MinHeight);
    return result;
  }
  const float a = MinimumExtent(*node.ChildA, axis);
  const float b = MinimumExtent(*node.ChildB, axis);
  return node.Axis == axis ? a + b : (std::max)(a, b);
}

} // namespace

bool DockRect::Contains(float x, float y) const {
  return x >= Left && y >= Top && x <= Left + Width && y <= Top + Height;
}

std::unique_ptr<DockNode> DockTree::CreateLeaf(BaseNode *panel) {
  auto leaf = std::make_unique<DockNode>();
  leaf->ID = NextID++;
  if (panel)
    leaf->Panels.push_back(panel);
  return leaf;
}

DockNode *DockTree::AddPanel(BaseNode *panel) {
  if (!panel)
    return nullptr;
  if (auto *existing = FindPanelLeaf(panel))
    return existing;
  if (!Root) {
    Root = CreateLeaf(panel);
    return Root.get();
  }
  // 已有根时必须通过显式 Split 事务加入，否则 Leaf 会退化成
  // 没有标题交互的重叠 Panel 集合。
  return nullptr;
}

void DockTree::Clear() {
  Root.reset();
  NextID = 1;
}

DockNode *DockTree::Find(DockNodeID id) const {
  return id == 0 ? nullptr : FindNode(Root.get(), id);
}

DockNode *DockTree::FindLeafAt(float x, float y) const {
  return FindLeafAtNode(Root.get(), x, y);
}

DockNode *DockTree::FindPanelLeaf(const BaseNode *panel) const {
  if (auto *result = FindPanel(Root.get(), panel))
    return result;
  // 业务层仍可以 Panel 查询停靠位置；DockTree 内的实际项始终是
  // PanelGroup，这个解析不改变树的单一真值。
  const auto *panelNode = dynamic_cast<const PanelNode *>(panel);
  return panelNode && panelNode->Group
             ? FindPanel(Root.get(), panelNode->Group)
             : nullptr;
}

DockNode *DockTree::FindSplitterAt(float x, float y, float tolerance) const {
  return FindSplitter(Root.get(), x, y, tolerance);
}

std::unique_ptr<DockNode> *DockTree::OwnerSlot(DockNode *node) {
  if (!node)
    return nullptr;
  if (!node->Parent)
    return Root.get() == node ? &Root : nullptr;
  if (node->Parent->ChildA.get() == node)
    return &node->Parent->ChildA;
  if (node->Parent->ChildB.get() == node)
    return &node->Parent->ChildB;
  return nullptr;
}

void DockTree::CollapseEmptyLeaf(DockNodeID leafID) {
  auto *leaf = Find(leafID);
  if (!leaf || leaf->Type != DockNodeType::Leaf || !leaf->Panels.empty())
    return;
  if (leaf == Root.get()) {
    Root.reset();
    return;
  }
  auto *split = leaf->Parent;
  auto *splitSlot = OwnerSlot(split);
  if (!splitSlot)
    return;
  std::unique_ptr<DockNode> sibling =
      split->ChildA.get() == leaf ? std::move(split->ChildB)
                                  : std::move(split->ChildA);
  sibling->Parent = split->Parent;
  *splitSlot = std::move(sibling);
}

bool DockTree::RemovePanel(BaseNode *panel) {
  auto *leaf = FindPanelLeaf(panel);
  if (!leaf)
    return false;
  const auto iterator = std::find(leaf->Panels.begin(), leaf->Panels.end(), panel);
  leaf->Panels.erase(iterator);
  if (leaf->Panels.empty()) {
    const auto id = leaf->ID;
    CollapseEmptyLeaf(id);
  }
  return true;
}

bool DockTree::Commit(const DockTransaction &transaction) {
  if (!transaction.Panel)
    return false;
  if (transaction.TargetFloating)
    return RemovePanel(transaction.Panel);
  if (transaction.TargetSide == DockSide::Center)
    return false;

  auto *targetBefore = Find(transaction.TargetNode);
  if (!targetBefore || targetBefore->Type != DockNodeType::Leaf)
    return false;
  if (FindPanelLeaf(transaction.Panel) == targetBefore &&
      targetBefore->Panels.size() == 1) {
    // 唯一页签投向自己的边缘无法产生两个有效子树。将其视为
    // 结构 no-op，避免先移除后失去目标 Leaf 并意外重建根 ID。
    return true;
  }

  RemovePanel(transaction.Panel);
  auto *target = Find(transaction.TargetNode);
  if (!target || target->Type != DockNodeType::Leaf) {
    // Source 折叠可能使目标 Leaf 晋升，但 ID 保持稳定；完全消失只可能是把
    // Panel 投回自己的空 Leaf，此时重新建立根即可保持唯一归属。
    if (!Root) {
      Root = CreateLeaf(transaction.Panel);
      return true;
    }
    return false;
  }
  auto *slot = OwnerSlot(target);
  if (!slot)
    return false;
  auto oldTarget = std::move(*slot);
  auto newLeaf = CreateLeaf(transaction.Panel);
  auto split = std::make_unique<DockNode>();
  split->ID = NextID++;
  split->Type = DockNodeType::Split;
  split->Parent = oldTarget->Parent;
  split->Axis = transaction.TargetSide == DockSide::Left ||
                        transaction.TargetSide == DockSide::Right
                    ? SplitAxis::Vertical
                    : SplitAxis::Horizontal;
  // 结构层只排除 0/1 退化值；像 ToolBar 这样的固定边缘控件可以合法
  // 小于工作区的 5%，具体像素最小值由节点样式和 ResizeSplitter 维护。
  split->SplitRatio = std::clamp(transaction.SplitRatio, 0.001f, 0.999f);
  const bool before = transaction.TargetSide == DockSide::Left ||
                      transaction.TargetSide == DockSide::Top;
  split->FixedExtent = (std::max)(0.0f, transaction.FixedExtent);
  split->FixedChild = split->FixedExtent > 0.0f
                          ? (before ? FixedSplitChild::A : FixedSplitChild::B)
                          : FixedSplitChild::None;
  split->SplitterResizable = transaction.SplitterResizable;
  split->ChildA = before ? std::move(newLeaf) : std::move(oldTarget);
  split->ChildB = before ? std::move(oldTarget) : std::move(newLeaf);
  split->ChildA->Parent = split.get();
  split->ChildB->Parent = split.get();
  *slot = std::move(split);
  return Validate();
}

void DockTree::LayoutNode(DockNode &node, const DockRect &rect) {
  node.Rect = rect;
  if (node.Type == DockNodeType::Leaf)
    return;
  const float extent = node.Axis == SplitAxis::Vertical ? rect.Width
                                                        : rect.Height;
  if (node.FixedChild != FixedSplitChild::None && extent > 0.0f) {
    // 固定边缘占用像素而不是比例；窗口 resize 时只让工作区吸收差值，避免
    // ToolBar 高度随客户区同比放大或缩小。
    const float fixed = std::clamp(node.FixedExtent, 0.0f, extent);
    node.SplitRatio = node.FixedChild == FixedSplitChild::A
                          ? fixed / extent
                          : 1.0f - fixed / extent;
  }
  node.SplitRatio = std::clamp(node.SplitRatio, 0.001f, 0.999f);
  DockRect a = rect;
  DockRect b = rect;
  if (node.Axis == SplitAxis::Vertical) {
    a.Width = rect.Width * node.SplitRatio;
    b.Left = rect.Left + a.Width;
    b.Width = rect.Width - a.Width;
  } else {
    a.Height = rect.Height * node.SplitRatio;
    b.Top = rect.Top + a.Height;
    b.Height = rect.Height - a.Height;
  }
  LayoutNode(*node.ChildA, a);
  LayoutNode(*node.ChildB, b);
}

void DockTree::Layout(const DockRect &workspace) {
  if (Root)
    LayoutNode(*Root, workspace);
}

DockRect DockTree::GetPreviewRect(const DockNode &target, DockSide side) const {
  DockRect result = target.Rect;
  constexpr float previewRatio = 0.3f;
  if (side == DockSide::Left || side == DockSide::Right)
    result.Width *= previewRatio;
  else if (side == DockSide::Top || side == DockSide::Bottom)
    result.Height *= previewRatio;
  // 右侧和下方仍只显示 30% 的预览尺寸，但其起点必须落在目标
  // 尺寸的 70% 处；若直接累加缩小后的尺寸，预览会错误地停在中间。
  if (side == DockSide::Right)
    result.Left += target.Rect.Width - result.Width;
  if (side == DockSide::Bottom)
    result.Top += target.Rect.Height - result.Height;
  return result;
}

bool DockTree::ResizeSplitter(DockNodeID id, float clientX, float clientY,
                              float minimumExtent) {
  auto *split = Find(id);
  if (!split || split->Type != DockNodeType::Split ||
      !split->SplitterResizable)
    return false;
  const float extent = split->Axis == SplitAxis::Vertical ? split->Rect.Width
                                                          : split->Rect.Height;
  if (extent <= 0.0f)
    return false;
  const float coordinate = split->Axis == SplitAxis::Vertical
                               ? clientX - split->Rect.Left
                               : clientY - split->Rect.Top;
  const float minimumA = (std::max)(minimumExtent,
                                    MinimumExtent(*split->ChildA, split->Axis));
  const float minimumB = (std::max)(minimumExtent,
                                    MinimumExtent(*split->ChildB, split->Axis));
  const float minRatio = (std::min)(0.5f, minimumA / extent);
  const float maxRatio = (std::max)(0.5f, 1.0f - minimumB / extent);
  split->SplitRatio = std::clamp(coordinate / extent, minRatio, maxRatio);
  return true;
}

bool DockTree::Validate(std::string *error) const {
  std::unordered_set<const BaseNode *> panels;
  const auto fail = [error](const std::string &message) {
    if (error)
      *error = message;
    return false;
  };
  const auto validate = [&](const auto &self, const DockNode *node,
                            const DockNode *parent) -> bool {
    if (!node)
      return fail("DockTree contains a null node.");
    if (node->Parent != parent)
      return fail("DockTree parent link is inconsistent.");
    if (node->Type == DockNodeType::Split) {
      if (!node->ChildA || !node->ChildB || !node->Panels.empty())
        return fail("Split node must own exactly two children and no panels.");
      if (!(node->SplitRatio > 0.0f && node->SplitRatio < 1.0f))
        return fail("Split ratio is outside the legal range.");
      return self(self, node->ChildA.get(), node) &&
             self(self, node->ChildB.get(), node);
    }
    if (node->ChildA || node->ChildB || node->Panels.size() != 1)
      return fail("Leaf node must own exactly one dock item and no children.");
    for (const auto *panel : node->Panels)
      if (!panel || !panels.insert(panel).second)
        return fail("Panel belongs to more than one Dock leaf.");
    return true;
  };
  return !Root || validate(validate, Root.get(), nullptr);
}

std::string DockTree::Dump() const {
  if (!Root)
    return "DockTree <empty>\n";
  std::ostringstream stream;
  stream << "DockTree Root #" << Root->ID << '\n';
  DumpNode(*Root, stream, "");
  return stream.str();
}
