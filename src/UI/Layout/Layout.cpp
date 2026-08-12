//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/Layout.h"

#include "Object/UIObject/UIObject.h"
#include "UI/Behavior/DockLayoutBehavior.h"
#include "UI/Layout/BaseNode.h"
#include "UI/Layout/DrawNode.h"
#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"

#include <algorithm>
#include <ostream>

using namespace z8;
using namespace z8::ui;

Layout::Layout(): Root(std::make_unique<BaseNode>()) {
  Root->Key = "Root";
  Root->AddBehavior<DockLayoutBehavior>();
  RebuildIndex();
}

Layout::~Layout() = default;

void Layout::SetRoot(std::unique_ptr<BaseNode> root) {
  Root = std::move(root);
  if (Root && !Root->GetBehavior<DockLayoutBehavior>())
    Root->AddBehavior<DockLayoutBehavior>();
  RebuildIndex();
}

void Layout::IndexTree(BaseNode *node) {
  if (!node)
    return;
  Nodes.push_back(node);
  // RTTI 只在拓扑变化时使用；布局热路径直接调用窄虚接口。
  if (auto *visual = dynamic_cast<DrawNode *>(node))
    Visuals.push_back(visual);
  for (const auto &child : node->Children)
    IndexTree(child.get());
}

void Layout::CancelTreeCaptures(BaseNode *node) {
  if (!node)
    return;
  // 从当前仍存活的树遍历，而不是解引用旧索引中的捕获指针；声明式协调可能
  // 已经删除旧 target，但被保留节点上的状态机仍需要收到 capture-lost。
  node->CancelPointerCapture();
  for (const auto &child : node->Children)
    CancelTreeCaptures(child.get());
}

void Layout::RebuildIndex() {
  // 声明式重建可能销毁旧节点，不能让指针捕获跨越拓扑变化。
  CancelTreeCaptures(Root.get());
  CapturedTarget = nullptr;
  CapturedHandler = nullptr;
  Nodes.clear();
  Visuals.clear();
  IndexTree(Root.get());
  Dirty = true;
}

// 返回命中的最上层 VisualNode
BaseNode *Layout::HitAt(float x, float y) const {
  // Visuals 按树的前序遍历，逆序就是最上层优先。
  for (auto I = Visuals.rbegin(); I != Visuals.rend(); ++I) {
    auto *visual = *I;
    if (visual->Contains(x, y))
      return visual;
  }
  return nullptr;
}

z8::MouseCursor Layout::GetMouseCursor(MouseMovArgs args) const {
  if (CapturedHandler) {
    const auto cursor = CapturedHandler->QueryMouseCursor(args);
    if (cursor != MouseCursor::Arrow)
      return cursor;
  }
  for (auto *node = HitAt(static_cast<float>(args.X),
                            static_cast<float>(args.Y));
       node;
       node = node->Parent) {
    const auto cursor = node->QueryMouseCursor(args);
    if (cursor != MouseCursor::Arrow)
      return cursor;
  }
  return MouseCursor::Arrow;
}

EventReply Layout::OnMouseDown(MouseMovArgs args) {
  auto *target =
      HitAt(static_cast<float>(args.X), static_cast<float>(args.Y));
  if (!target) return EventReply::Ignored;

  CapturedTarget = target;
  // 输入只属于节点和 Behavior；UIObject 是渲染数据，不再接收重复事件。
  for (auto *node = target; node; node = node->Parent) {
    const auto reply = node->DispatchMouseDown(args);
    if (reply == EventReply::Capture)
      CapturedHandler = node;
    if (reply != EventReply::Ignored)
      break;
  }
  // 命中任何 UI 都会阻止事件穿透到场景，即使控件没有主动手势。
  return EventReply::Handled;
}

EventReply Layout::OnMouseMove(MouseMovArgs args) {
  auto *target =
      HitAt(static_cast<float>(args.X), static_cast<float>(args.Y));
  if (!target)
    return EventReply::Ignored;
  for (auto *node = target; node; node = node->Parent)
    if (node->DispatchMouseMove(args))
      break;
  return EventReply::Handled;
}

EventReply Layout::OnMouseDrag(MouseMovArgs args) {
  if (!CapturedTarget)
    return EventReply::Ignored;
  if (CapturedHandler)
    CapturedHandler->DispatchMouseDrag(args);
  return EventReply::Handled;
}

EventReply Layout::OnMouseUp(MouseMovArgs args) {
  if (!CapturedTarget)
    return EventReply::Ignored;
  if (CapturedHandler)
    CapturedHandler->DispatchMouseUp(args);
  CapturedTarget = nullptr;
  CapturedHandler = nullptr;
  return EventReply::Handled;
}

EventReply Layout::OnMouseWheel(MouseWheelArgs args) {
  auto *target =
      HitAt(static_cast<float>(args.X), static_cast<float>(args.Y));
  if (!target)
    return EventReply::Ignored;
  for (auto *node = target; node; node = node->Parent)
    if (node->DispatchMouseWheel(args))
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
}

BaseNode *Layout::Find(const std::string &key) const {
  for (auto *node : Nodes)
    if (node->Key == key)
      return node;
  return nullptr;
}

bool Layout::ConsumeDirty() {
  const bool result = Dirty;
  Dirty = false;
  return result;
}

std::vector<GameObject *> Layout::GetUO() const {
  std::vector<GameObject *> result;
  result.reserve(Visuals.size());
  for (auto *v : Visuals) {
    assert (v->UO);
    result.push_back(v->UO.get());
  }
  return result;
}

// 每帧计算布局的入口函数
void Layout::Calculate(float w, float h) {
  Root->DispatchBeforeLayout(w, h);
  YGNodeCalculateLayout(Root->Node, w, h, YGDirectionLTR);
  UpdateTree(Root->Node, 0, 0, {0.0f, 0.0f, w, h});
}

void Layout::UpdateTree(YGNodeRef Node, float parentX, float parentY,
                        const DirectX::XMFLOAT4 &clip) {
  auto N = static_cast<BaseNode *>(YGNodeGetContext(Node));

  // 1. 这里得到的是相对于父容器的位置
  float x = YGNodeLayoutGetLeft(Node);
  float y = YGNodeLayoutGetTop(Node);
  float width = YGNodeLayoutGetWidth(Node);
  float height = YGNodeLayoutGetHeight(Node);

  // 2. 然后计算绝对位置
  float absX = parentX + x;
  float absY = parentY + y;

  // 3. 记录本次数据，并更新
  N->Left = absX;
  N->Top = absY;
  N->Width = width;
  N->Height = height;
  N->VisibleClip = clip;
  N->Synchronize();

  // 4. 计算裁剪矩形
  DirectX::XMFLOAT4 childClip = clip;
  if (N->ClipChildren) {
    childClip.x = (std::max)(childClip.x, absX);
    childClip.y = (std::max)(childClip.y, absY);
    childClip.z = (std::min)(childClip.z, absX + width);
    childClip.w = (std::min)(childClip.w, absY + height);
  }

  // 5. 迭代子节点
  for (size_t i = 0; i < N->Children.size(); ++i) {
    YGNodeRef child = YGNodeGetChild(Node, i);
    UpdateTree(child, absX + N->ChildOffsetX, absY + N->ChildOffsetY,
               childClip);
  }

  // 6. 事件通知
  N->DispatchLayoutUpdated();
}
