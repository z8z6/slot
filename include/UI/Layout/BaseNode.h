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
  // 布局结果使用窗口客户区绝对坐标，渲染与命中测试共享这些字段。
  float Left = 0.0f;
  float Top = 0.0f;
  float Width = 0.0f;
  float Height = 0.0f;
  float ChildOffsetX = 0.0f;
  float ChildOffsetY = 0.0f;
  bool ClipChildren = false;
  bool Visible = true;
  ClipRect VisibleClip = {-100000.0f, -100000.0f, 100000.0f,
                                   100000.0f};
  GSL_OWNER YGNodeRef Node;
  BaseNode *Parent = nullptr;
  std::vector<std::unique_ptr<BaseNode>> Children;

  // Key 是声明式 UI 在多次构建之间复用控件的稳定身份。
  std::string Key;

public:
  BaseNode();
  virtual ~BaseNode();
  BaseNode(const BaseNode &) = delete;
  BaseNode &operator=(const BaseNode &) = delete;

  virtual const char *TypeName() const;
  bool SetProperty(const std::string &name, const std::string &value) override;
  virtual BaseNode *ContentHost();
  bool Contains(float x, float y) const;
  bool Contains(MouseMovArgs args) const;
  BaseNode *AddChild(std::unique_ptr<BaseNode> child);
  void RemoveChildrenFrom(size_t first);

  /** 子树完成布局后调用，复合控件在此计算滚动范围等派生几何。 */
  virtual void OnLayoutUpdated() {}
  virtual void Synchronize() {}
  void DispatchLayoutUpdated();
};
} // namespace z8::ui
