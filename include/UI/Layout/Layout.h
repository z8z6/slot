//
// Created by zhou_zhengming on 2026/7/29.
//

#pragma once
#include "Core/Event.h"
#include "yoga/YGNode.h"
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
class DrawNode;

/**
 * UI 树的布局与输入路由器。
 */
class Layout : public EventTarget {
public:
  std::unique_ptr<BaseNode> Root;
  std::vector<BaseNode *> Nodes;
  std::vector<DrawNode *> Visuals;

  bool Dirty = true;
  BaseNode *CapturedTarget = nullptr;
  BaseNode *CapturedHandler = nullptr;

  explicit Layout();
  ~Layout() override;

  void RebuildIndex();
  BaseNode *Find(const std::string &key) const;
  void Calculate(float width, float height);
  void MarkDirty() { Dirty = true; }
  bool ConsumeDirty();
  void SetRoot(std::unique_ptr<BaseNode> root);
  std::vector<GameObject *> GetUO() const;

  EventReply OnMouseDown(MouseMovArgs args) override;
  EventReply OnMouseMove(MouseMovArgs args) override;
  EventReply OnMouseDrag(MouseMovArgs args) override;
  EventReply OnMouseUp(MouseMovArgs args) override;
  EventReply OnMouseWheel(MouseWheelArgs args) override;
  MouseCursor GetMouseCursor(MouseMovArgs args) const override;
  void OnPointerCaptureLost() override;

private:
  void IndexTree(BaseNode *node);
  void CancelTreeCaptures(BaseNode *node);
  BaseNode *HitAt(float x, float y) const;
  void UpdateTree(YGNodeRef Node, float parentX, float parentY,
                  const DirectX::XMFLOAT4 &clip);
};
} // namespace z8::ui
