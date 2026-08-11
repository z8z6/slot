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
public:
  Application* App;
  std::vector<BaseNode*> Nodes;
  std::vector<GameObject*> UOs;
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

private:
  std::unique_ptr<BaseNode> RootOwner;
  bool TopologyDirty = true;
  BaseNode* CapturedTarget = nullptr;
  BaseNode* CapturedHandler = nullptr;
  void IndexTree(BaseNode* node);
  BaseNode* HitTest(float x, float y) const;
  void UpdateTree(YGNodeRef Node, float parentX, float parentY,
                  const DirectX::XMFLOAT4& clip);
};
}





