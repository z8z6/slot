//
// Created by zhou_zhengming on 2026/7/29.
//

#include "UI/Layout/Layout.h"

#include "Object/UIObject/RectUIObject.h"
#include "UI/Layout/RectNode.h"
#include "yoga/YGNodeLayout.h"
#include "yoga/YGNodeStyle.h"

#include <algorithm>
#include <iostream>
#include <ostream>

using namespace z8;
using namespace z8::ui;

Layout::Layout(Application *App) : App(App), Root(nullptr) {
  auto root = std::make_unique<BaseNode>();
  root->Key = "Root";
  SetRoot(std::move(root));
}

Layout::~Layout() = default;

void Layout::SetRoot(std::unique_ptr<BaseNode> root) {
  RootOwner = std::move(root);
  Root = RootOwner.get();
  RebuildIndex();
}

void Layout::IndexTree(BaseNode *node) {
  if (!node)
    return;
  Nodes.push_back(node);
  for (const auto &child : node->GetChildren())
    IndexTree(child.get());
}

void Layout::CancelTreeCaptures(BaseNode *node) {
  if (!node)
    return;
  // 从当前仍存活的树遍历，而不是解引用旧索引中的捕获指针；声明式协调可能
  // 已经删除旧 target，但被保留节点上的状态机仍需要收到 capture-lost。
  node->CancelPointerCapture();
  for (const auto &child : node->GetChildren())
    CancelTreeCaptures(child.get());
}

void Layout::RebuildIndex() {
  // 声明式重建可能销毁旧节点，不能让指针捕获跨越拓扑变化。
  CancelTreeCaptures(Root);
  CapturedTarget = nullptr;
  CapturedHandler = nullptr;
  Nodes.clear();
  IndexTree(Root);
  TopologyDirty = true;
}

BaseNode *Layout::HitTest(float x, float y) const {
  // UI batch 按树的前序绘制，逆序就是最上层视觉优先。
  for (auto iterator = Nodes.rbegin(); iterator != Nodes.rend(); ++iterator) {
    auto *node = *iterator;
    if (node->GetUO() && node->Contains(x, y))
      return node;
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
  target->GetUO()->OnMouseDown(args);
  for (auto *node = target; node; node = node->Parent) {
    const auto reply = node->DispatchMouseDown(args);
    if (reply == UIEventReply::Capture)
      CapturedHandler = node;
    if (reply != UIEventReply::Ignored)
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
  target->GetUO()->OnMouseMove(args);
  return true;
}

bool Layout::OnMouseDrag(MouseMovArgs args) {
  if (!CapturedTarget)
    return false;
  CapturedTarget->GetUO()->OnMouseMove(args);
  CapturedTarget->GetUO()->OnMouseDrag(args);
  if (CapturedHandler)
    CapturedHandler->DispatchMouseDrag(args);
  return true;
}

bool Layout::OnMouseUp(MouseMovArgs args) {
  if (!CapturedTarget)
    return false;
  CapturedTarget->GetUO()->OnMouseUp(args);
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

std::vector<z8::GameObject *> Layout::GetUO() const {
  std::vector<GameObject *> result;
  result.reserve(Nodes.size());
  for (auto *node : Nodes) {
    // Root、Viewport 和 Content 等节点只参与布局，没有渲染对象；旧 UOs 索引
    // 会在建树时过滤它们，新按需接口必须保持相同契约，避免下游直接解引用空值。
    if (auto *object = node->GetUO())
      result.push_back(object);
  }
  return result;
}

void Layout::Calculate(float w, float h) {
  // 计算整体布局
  YGNodeCalculateLayout(Root->GetYogaNode(), w, h, YGDirectionLTR);
  // 更新节点树
  UpdateTree(Root->GetYogaNode(), 0, 0, {0.0f, 0.0f, w, h});
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

  // 事件系统使用与渲染完全相同的绝对布局框，避免命中区域与画面错位。
  N->LayoutX = absX;
  N->LayoutY = absY;
  N->LayoutWidth = width;
  N->LayoutHeight = height;
  N->VisibleClip = clip;

  // 应用位置和大小
  if (N->GetUO()) {
    N->GetUO()->SetPosition(absX, absY, width, height);
    N->GetUO()->SetScale(width, height);
    N->GetUO()->SetClipRect(N->Visible ? clip : DirectX::XMFLOAT4{0, 0, 0, 0});
  }

  DirectX::XMFLOAT4 childClip = clip;
  if (N->ClipsChildren) {
    childClip.x = (std::max)(childClip.x, absX);
    childClip.y = (std::max)(childClip.y, absY);
    childClip.z = (std::min)(childClip.z, absX + width);
    childClip.w = (std::min)(childClip.w, absY + height);
  }

  for (size_t i = 0; i < N->GetChildSize(); ++i) {
    YGNodeRef child = YGNodeGetChild(Node, i);
    UpdateTree(child, absX + N->ChildOffsetX, absY + N->ChildOffsetY,
               childClip);
  }
  N->DispatchLayoutUpdated();
}
