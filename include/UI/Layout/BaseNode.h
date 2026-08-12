//
// Created by zhou_zhengming on 2026/7/29.
//

#pragma once
#include "Core/Event.h"
#include "UI/Property/IProperty.h"
#include "Util/Owner.h"
#include "yoga/YGNode.h"

#include <DirectXMath.h>

#include <memory>
#include <string>
#include <vector>

namespace z8::ui {
/**
 * 保留式 UI 树的非视觉基础节点，同时也是所有声明属性的根接口实现。
 *
 * BaseNode 只维护 Yoga 几何、树关系和裁剪传播，不拥有渲染或交互状态。
 * Root、Viewport、Content 等结构节点因此不会伪装成可绘制对象；需要绘制的
 * 控件继承 DrawNode，需要事件/Behavior 的节点显式继承 BehaviorNode。
 */
using ClipRect = DirectX::XMFLOAT4;
class BaseNode : public virtual IProperty {
public:
  std::string Key;
  GSL_OWNER YGNodeRef Node;
  BaseNode *Parent = nullptr;
  std::vector<std::unique_ptr<BaseNode>> Children;

  float Left = 0.0f;
  float Top = 0.0f;
  float Width = 0.0f;
  float Height = 0.0f;
  float ChildOffsetX = 0.0f;
  float ChildOffsetY = 0.0f;
  bool ClipChildren = false;
  bool Visible = true;
  ClipRect VisibleClip = {-100000.0f, -100000.0f, 100000.0f, 100000.0f};

public:
  BaseNode();
  ~BaseNode() override;
  BaseNode(const BaseNode &) = delete;
  BaseNode &operator=(const BaseNode &) = delete;

  virtual const char *TypeName() const;
  /** 标记该节点是否只是场景输入窗口；默认 UI 节点会阻止事件继续传播。 */
  virtual bool RoutesToScene() const { return false; }
  bool SetProperty(const std::string &name, const std::string &value) override;
  virtual BaseNode *ContentHost();
  bool Contains(float x, float y) const;
  bool Contains(MouseMovArgs args) const;
  BaseNode *AddChild(std::unique_ptr<BaseNode> child);
  void RemoveChildrenFrom(size_t first);

  virtual void OnAfterLayout() {}
  virtual void Synchronize() {}
  virtual void DispatchAfterLayout();
};
} // namespace z8::ui
