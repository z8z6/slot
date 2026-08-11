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
protected:
  // GSL_OWNER 明确需要释放
  GSL_OWNER YGNodeRef YogaNode;
  std::unique_ptr<UIObject> UO;
  std::vector<std::unique_ptr<BaseNode>> ChildNodes;

public:
  // Key 是声明式 UI 在多次构建之间复用控件的稳定身份。
  std::string Key;
  // Parent 不拥有指针，所有权位于父节点的 ChildNodes
  BaseNode *Parent = nullptr;

  BaseNode();
  virtual ~BaseNode();

  BaseNode(const BaseNode &) = delete;
  BaseNode &operator=(const BaseNode &) = delete;

  size_t GetChildCount() const;
  BaseNode *GetChild(size_t index) const;
  YGNodeRef GetYogaNode() const { return YogaNode; }
  UIObject *GetUO() const { return UO.get(); }
  float GetLayoutX() const { return LayoutX; }
  float GetLayoutY() const { return LayoutY; }
  float GetLayoutWidth() const { return LayoutWidth; }
  float GetLayoutHeight() const { return LayoutHeight; }

  // 鼠标是否命中
  bool Contains(float x, float y) const;
  bool Contains(MouseMovArgs args) const;

  /**
   * UI 指针事件沿视觉节点向父级冒泡；返回 true 表示节点开始一次捕获手势。
   * 捕获后拖拽和抬起直接送回该节点，避免指针越界时丢失交互状态。
   */
  virtual bool OnMouseDown(MouseMovArgs) { return false; }
  virtual bool OnMouseDrag(MouseMovArgs) { return false; }
  virtual bool OnMouseUp(MouseMovArgs) { return false; }
  virtual bool OnMouseWheel(MouseWheelArgs) { return false; }
  /** 子树完成布局后调用，复合控件在此计算滚动范围等派生几何。 */
  virtual void OnLayoutUpdated() {}
  // 返回当前位置期望的指针形状，父级可为子视觉提供边界光标
  virtual MouseCursor GetMouseCursor(MouseMovArgs) const {
    return MouseCursor::Arrow;
  }

  // 控件树和 Yoga 树必须同步修改。
  BaseNode *AddChild(std::unique_ptr<BaseNode> child);
  // 移除索引之后的所有子节点
  void RemoveChildrenFrom(size_t first);

  // 表示该容器如果要添加子元素，其合法的父容器
  // 容器可把声明的子控件重定向到内部内容宿主，Panel 用它隔离标题栏。
  virtual BaseNode *ContentHost();
  virtual const char *TypeName() const;
  bool SetProperty(const std::string &name,
                   const std::string &value) override;
  /** 对带视觉对象的控件统一设置颜色；纯布局容器返回 false。 */
  bool SetColor(const DirectX::XMFLOAT4 &color);
  void SetChildOffset(float x, float y) {
    ChildOffsetX = x;
    ChildOffsetY = y;
  }
  void SetClipsChildren(bool clips) { ClipsChildren = clips; }
  void SetVisible(bool visible) { Visible = visible; }
  bool IsVisible() const { return Visible; }

  const std::vector<std::unique_ptr<BaseNode>> &GetChildren() const {
    return ChildNodes;
  }

protected:
  void SetObject(std::unique_ptr<UIObject> object);

private:
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
  DirectX::XMFLOAT4 VisibleClip = {-100000.0f, -100000.0f,
                                   100000.0f, 100000.0f};
};
} // namespace z8::ui
