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

namespace z8::ui {
/**
 * 保留式 UI 树的非视觉基础节点，同时也是所有声明属性的根接口实现。
 *
 * BaseNode 只维护 Yoga 几何、树关系、输入路由和 Behavior，不拥有渲染对象。
 * Root、Viewport、Content 等结构节点因此不会伪装成可绘制对象；需要绘制的
 * 控件继承 VisualNode，由后者独占 UIObject。
 */
class BaseNode : public virtual IProperty, public EventTarget {
public:
  // 布局结果使用窗口客户区绝对坐标，渲染与命中测试共享这些字段。
  float Left = 0.0f;
  float Top = 0.0f;
  float Width = 0.0f;
  float Height = 0.0f;
  float ChildOffsetX = 0.0f;
  float ChildOffsetY = 0.0f;
  bool ClipsChildren = false;
  bool Visible = true;
  DirectX::XMFLOAT4 VisibleClip = {-100000.0f, -100000.0f, 100000.0f,
                                   100000.0f};
  // Node 由 BaseNode 独占；Children 与 Yoga 子树始终按相同顺序更新。
  GSL_OWNER YGNodeRef Node;
  BaseNode *Parent = nullptr;
  std::vector<std::unique_ptr<BaseNode>> Children;
  // Behavior 生命周期严格短于宿主，内部可安全观察宿主和复合视觉子节点。
  std::vector<std::unique_ptr<UIBehavior>> Behaviors;
  UIBehavior *CapturedBehavior = nullptr;

  // Key 是声明式 UI 在多次构建之间复用控件的稳定身份。
  std::string Key;

  BaseNode();
  virtual ~BaseNode();
  BaseNode(const BaseNode &) = delete;
  BaseNode &operator=(const BaseNode &) = delete;

  bool Contains(float x, float y) const;
  bool Contains(MouseMovArgs args) const;

  /** 路由器放弃一次捕获时，旧式控件用它清理未完成的手势状态。 */
  virtual void OnPointerCaptureLost() {}
  /** 子树完成布局后调用，复合控件在此计算滚动范围等派生几何。 */
  virtual void OnLayoutUpdated() {}
  /**
   * 将缓存布局提交给可选视觉；非视觉节点默认无操作。
   *
   * 该窄接口避免 BaseNode 依赖 UIObject，也避免 Layout 在每帧遍历中使用 RTTI。
   */
  virtual void SynchronizeVisual(const DirectX::XMFLOAT4 &) {}
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
  /** 以下入口只供 Layout 路由器调用，统一组合 Behavior 与节点事件钩子。 */
  EventReply DispatchMouseDown(MouseMovArgs args);
  bool DispatchMouseMove(MouseMovArgs args);
  bool DispatchMouseDrag(MouseMovArgs args);
  bool DispatchMouseUp(MouseMovArgs args);
  bool DispatchMouseWheel(MouseWheelArgs args);
  MouseCursor QueryMouseCursor(MouseMovArgs args) const;
  void DispatchLayoutUpdated();
  /** 在 Yoga 测量前允许容器行为写入本轮布局约束。 */
  void DispatchBeforeLayout(float width, float height);
  /** 广播有效拖拽边界，让停靠等兄弟行为无需依赖挂载顺序或回调所有权。 */
  void DispatchDragStarted(MouseMovArgs args);
  void DispatchDragCompleted(MouseMovArgs args);
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

protected:
  /**
   * 提前释放观察宿主资源的 Behavior；VisualNode 在视觉成员析构前调用。
   *
   * BaseNode 析构会再次安全调用，因此纯布局节点和构造失败路径无需特殊处理。
   */
  void ReleaseBehaviors();
};
} // namespace z8::ui
