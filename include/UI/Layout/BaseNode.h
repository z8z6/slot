//
// Created by zhou_zhengming on 2026/7/29.
//

#pragma once
#include "Core/Event.h"
#include "UI/Behavior/UIBehavior.h"
#include "UI/Property/IProperty.h"
#include "Util/Owner.h"
#include "yoga/YGNode.h"

#include <DirectXMath.h>

#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace z8 {
class UIObject;
}

namespace z8::ui {
/**
 * 保留式 UI 树的基础节点，同时也是所有声明属性的根接口实现。
 *
 * XAML、即时声明和检查器只依赖 IProperty，不需要识别具体控件；拖拽、
 * 拉伸和滚动等可选能力再由派生类通过更窄的接口显式声明。
 */
class BaseNode : public virtual IProperty {
  friend class Layout;
  // 缓存控件的绝对位置和大小
  float LayoutX = 0.0f;
  float LayoutY = 0.0f;
  float LayoutWidth = 0.0f;
  float LayoutHeight = 0.0f;
  float ChildOffsetX = 0.0f;
  float ChildOffsetY = 0.0f;
  bool ClipsChildren = false;
  bool Visible = true;
  DirectX::XMFLOAT4 VisibleClip = {-100000.0f, -100000.0f, 100000.0f,
                                   100000.0f};
protected:
  GSL_OWNER YGNodeRef Node;
  std::unique_ptr<UIObject> UO;
  BaseNode *Parent;
  std::vector<std::unique_ptr<BaseNode>> Children;
  std::vector<std::unique_ptr<UIBehavior>> Behaviors;
  UIBehavior *CapturedBehavior = nullptr;

public:
  // Key 是声明式 UI 在多次构建之间复用控件的稳定身份。
  std::string Key;

  BaseNode();
  virtual ~BaseNode();
  BaseNode(const BaseNode &) = delete;
  BaseNode &operator=(const BaseNode &) = delete;

  size_t GetChildSize() const { return Children.size(); }
  BaseNode *GetChild(size_t index) const {
    return index < GetChildSize() ? Children[index].get() : nullptr;
  }
  const std::vector<std::unique_ptr<BaseNode>> &GetChildren() const {
    return Children;
  }
  YGNodeRef GetYogaNode() const { return Node; }
  UIObject *GetUO() const { return UO.get(); }
  float GetLayoutX() const { return LayoutX; }
  float GetLayoutY() const { return LayoutY; }
  float GetLayoutWidth() const { return LayoutWidth; }
  float GetLayoutHeight() const { return LayoutHeight; }
  bool Contains(float x, float y) const;
  bool Contains(MouseMovArgs args) const;

  virtual bool OnMouseDown(MouseMovArgs) { return false; }
  virtual bool OnMouseDrag(MouseMovArgs) { return false; }
  virtual bool OnMouseUp(MouseMovArgs) { return false; }
  virtual bool OnMouseWheel(MouseWheelArgs) { return false; }
  /** 路由器放弃一次捕获时，旧式控件用它清理未完成的手势状态。 */
  virtual void OnPointerCaptureLost() {}
  /** 子树完成布局后调用，复合控件在此计算滚动范围等派生几何。 */
  virtual void OnLayoutUpdated() {}
  // 返回当前位置期望的指针形状，父级可为子视觉提供边界光标
  virtual MouseCursor GetMouseCursor(MouseMovArgs) const {
    return MouseCursor::Arrow;
  }

  /**
   * 添加一个可组合行为并返回稳定观察指针。
   *
   * 行为按优先级降序调度；同优先级保持添加顺序，从而让控件组装代码显式决定
   * 冲突策略，而不是在控件事件函数里不断增加 if/else。行为集合只能在事件
   * 分发之外修改，以保证当前回调地址稳定。
   */
  UIBehavior *AddBehavior(std::unique_ptr<UIBehavior> behavior);
  template <typename T, typename... Args> T *AddBehavior(Args &&...args) {
    static_assert(std::is_base_of_v<UIBehavior, T>);
    auto behavior = std::make_unique<T>(std::forward<Args>(args)...);
    auto *observer = behavior.get();
    AddBehavior(std::move(behavior));
    return observer;
  }
  bool RemoveBehavior(UIBehavior *behavior);
  template <typename T> T *GetBehavior() {
    static_assert(std::is_base_of_v<UIBehavior, T>);
    for (auto &behavior : Behaviors)
      if (auto *result = dynamic_cast<T *>(behavior.get()))
        return result;
    return nullptr;
  }
  template <typename T> const T *GetBehavior() const {
    static_assert(std::is_base_of_v<UIBehavior, T>);
    for (const auto &behavior : Behaviors)
      if (auto *result = dynamic_cast<const T *>(behavior.get()))
        return result;
    return nullptr;
  }
  const std::vector<std::unique_ptr<UIBehavior>> &GetBehaviors() const {
    return Behaviors;
  }

  /** 以下入口只供 Layout 路由器调用，统一组合 Behavior 与旧控件事件钩子。 */
  UIEventReply DispatchMouseDown(MouseMovArgs args);
  bool DispatchMouseDrag(MouseMovArgs args);
  bool DispatchMouseUp(MouseMovArgs args);
  bool DispatchMouseWheel(MouseWheelArgs args);
  MouseCursor QueryMouseCursor(MouseMovArgs args) const;
  void DispatchLayoutUpdated();
  void CancelPointerCapture();

  // 控件树和 Yoga 树必须同步修改。
  BaseNode *AddChild(std::unique_ptr<BaseNode> child);
  // 移除索引之后的所有子节点
  void RemoveChildrenFrom(size_t first);

  // 表示该容器如果要添加子元素，其合法的父容器
  // 容器可把声明的子控件重定向到内部内容宿主，Panel 用它隔离标题栏。
  virtual BaseNode *ContentHost();
  virtual const char *TypeName() const;
  bool SetProperty(const std::string &name, const std::string &value) override;
  /** 对带视觉对象的控件统一设置颜色；纯布局容器返回 false。 */
  bool SetColor(const DirectX::XMFLOAT4 &color);
  void SetChildOffset(float x, float y) {
    ChildOffsetX = x;
    ChildOffsetY = y;
  }
  void SetClipsChildren(bool clips) { ClipsChildren = clips; }
  void SetVisible(bool visible) { Visible = visible; }
  bool IsVisible() const { return Visible; }

};
} // namespace z8::ui
