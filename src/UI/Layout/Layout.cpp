//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/Layout.h"

#include "Object/UIObject/UIObject.h"
#include "UI/Behavior/DockLayoutBehavior.h"
#include "UI/Layout/BaseNode.h"
#include "UI/Layout/VisualNode.h"
#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"

#include <algorithm>
#include <iostream>
#include <ostream>

using namespace z8;
using namespace z8::ui;

Layout::Layout(Application *App)
    : Root(std::make_unique<BaseNode>()), App(App) {
  Root->Key = "Root";
  // 根节点默认作为 DockSpace；无 DockBehavior 的普通子节点保持原 Yoga 布局。
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
  if (auto *visual = dynamic_cast<VisualNode *>(node))
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
  TopologyDirty = true;
}

BaseNode *Layout::HitTest(float x, float y) const {
  // UI batch 按树的前序绘制，逆序就是最上层视觉优先。
  for (auto iterator = Visuals.rbegin(); iterator != Visuals.rend();
       ++iterator) {
    auto *visual = *iterator;
    if (visual->Visual && visual->Contains(x, y))
      return visual;
  }
  return nullptr;
}

z8::MouseCursor Layout::GetMouseCursor(int x, int y) const {
  MouseMovArgs args;
  args.X = x;
  args.Y = y;
  if (CapturedHandler) {
    const auto cursor = CapturedHandler->QueryMouseCursor(args);
    if (cursor != MouseCursor::Arrow)
      return cursor;
  }
  for (auto *node = HitTest(static_cast<float>(x), static_cast<float>(y)); node;
       node = node->Parent) {
    const auto cursor = node->QueryMouseCursor(args);
    if (cursor != MouseCursor::Arrow)
      return cursor;
  }
  return MouseCursor::Arrow;
}

bool Layout::OnMouseDown(MouseMovArgs args) {
  auto *target =
      HitTest(static_cast<float>(args.X), static_cast<float>(args.Y));
  if (!target)
    return false;

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
  return true;
}

bool Layout::OnMouseMove(MouseMovArgs args) {
  auto *target =
      HitTest(static_cast<float>(args.X), static_cast<float>(args.Y));
  if (!target)
    return false;
  for (auto *node = target; node; node = node->Parent)
    if (node->DispatchMouseMove(args))
      break;
  return true;
}

bool Layout::OnMouseDrag(MouseMovArgs args) {
  if (!CapturedTarget)
    return false;
  if (CapturedHandler)
    CapturedHandler->DispatchMouseDrag(args);
  return true;
}

bool Layout::OnMouseUp(MouseMovArgs args) {
  if (!CapturedTarget)
    return false;
  if (CapturedHandler)
    CapturedHandler->DispatchMouseUp(args);
  CapturedTarget = nullptr;
  CapturedHandler = nullptr;
  return true;
}

bool Layout::OnMouseWheel(MouseWheelArgs args) {
  auto *target =
      HitTest(static_cast<float>(args.X), static_cast<float>(args.Y));
  if (!target)
    return false;
  for (auto *node = target; node; node = node->Parent)
    if (node->DispatchMouseWheel(args))
      return true;
  // 与其他指针事件一致，命中 UI 后不允许滚轮穿透到 3D 场景。
  return true;
}

BaseNode *Layout::Find(const std::string &key) const {
  for (auto *node : Nodes)
    if (node->Key == key)
      return node;
  return nullptr;
}

bool Layout::ConsumeTopologyDirty() {
  const bool result = TopologyDirty;
  TopologyDirty = false;
  return result;
}

std::vector<GameObject *> Layout::CollectVisualObjects() const {
  std::vector<GameObject *> result;
  result.reserve(Visuals.size());
  for (auto *visual : Visuals) {
    // VisualNode 构造时必须提供视觉；检查用于防御自定义节点错误转移所有权。
    if (visual->Visual)
      result.push_back(visual->Visual.get());
  }
  return result;
}

void Layout::Calculate(float w, float h) {
  Root->DispatchBeforeLayout(w, h);
  YGNodeCalculateLayout(Root->Node, w, h, YGDirectionLTR);
  UpdateTree(Root->Node, 0, 0, {0.0f, 0.0f, w, h});
}

void Layout::UpdateTree(YGNodeRef Node, float parentX, float parentY,
                        const DirectX::XMFLOAT4 &clip) {
  auto N = static_cast<BaseNode *>(YGNodeGetContext(Node));

  // 相对于父容器的位置
  float x = YGNodeLayoutGetLeft(Node);
  float y = YGNodeLayoutGetTop(Node);

  // 长宽
  float width = YGNodeLayoutGetWidth(Node);
  float height = YGNodeLayoutGetHeight(Node);

  // 计算绝对位置
  float absX = parentX + x;
  float absY = parentY + y;

  N->Left = absX;
  N->Top = absY;
  N->Width = width;
  N->Height = height;
  N->VisibleClip = clip;

  // 非视觉节点为空操作；VisualNode 在窄接口内同步位置、缩放与裁剪。
  N->SynchronizeVisual(clip);

  DirectX::XMFLOAT4 childClip = clip;
  if (N->ClipsChildren) {
    childClip.x = (std::max)(childClip.x, absX);
    childClip.y = (std::max)(childClip.y, absY);
    childClip.z = (std::min)(childClip.z, absX + width);
    childClip.w = (std::min)(childClip.w, absY + height);
  }

  for (size_t i = 0; i < N->Children.size(); ++i) {
    YGNodeRef child = YGNodeGetChild(Node, i);
    UpdateTree(child, absX + N->ChildOffsetX, absY + N->ChildOffsetY,
               childClip);
  }
  N->DispatchLayoutUpdated();
}
