//
// Created by zhou_zhengming on 2026/7/29.
//

#pragma once
#include "RectNode.h"

#include <memory>
#include <string>
#include <vector>

namespace z8 {
class Application;
class GameObject;
}

namespace z8::ui {
class BaseNode;
class Layout {
  std::unique_ptr<BaseNode> RootOwner;
  bool TopologyDirty = true;
  BaseNode* CapturedTarget = nullptr;
  BaseNode* CapturedHandler = nullptr;
public:
  Application* App;
  std::vector<BaseNode*> Nodes;
  BaseNode* Root;

  explicit Layout(Application* App);
  ~Layout();

  void SetRoot(std::unique_ptr<BaseNode> root);
  void RebuildIndex();
  BaseNode* Find(const std::string& key) const;
  void Update();
  void Calculate(float width, float height);

  /** 对 UI 做逆绘制顺序命中，并维持一次完整的按下—拖拽—抬起捕获。 */
  bool OnMouseDown(MouseMovArgs args);
  bool OnMouseMove(MouseMovArgs args);
  bool OnMouseDrag(MouseMovArgs args);
  bool OnMouseUp(MouseMovArgs args);
  bool OnMouseWheel(MouseWheelArgs args);
  /** 查询最上层命中控件及其父链所请求的系统指针形状。 */
  MouseCursor GetMouseCursor(int x, int y) const;

  void MarkTopologyDirty() { TopologyDirty = true; }
  bool ConsumeTopologyDirty();
  /**
   * 按视觉树绘制顺序返回实际存在的 UI 对象。
   *
   * 纯布局节点没有 UIObject，必须在边界处过滤，调用方才能安全地把结果直接
   * 交给渲染批次或事件广播，而不必重复进行空指针检查。
   */
  std::vector<GameObject *> GetUO() const;

private:

  void IndexTree(BaseNode* node);
  void CancelTreeCaptures(BaseNode* node);
  BaseNode* HitTest(float x, float y) const;
  void UpdateTree(YGNodeRef Node, float parentX, float parentY,
                  const DirectX::XMFLOAT4& clip);
};
}




