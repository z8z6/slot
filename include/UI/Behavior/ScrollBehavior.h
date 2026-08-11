#pragma once

#include "UI/Behavior/UIBehavior.h"
#include "UI/Property/IScrollable.h"

namespace z8::ui {

class BaseNode;
class ScrollBarNode;

/**
 * 连接 viewport、content 与可选滚动条的滚动行为。
 *
 * 三个节点均为非拥有观察指针，并应与行为宿主处于同一复合控件子树。行为只
 * 依赖 BaseNode 的几何/裁剪协议和 ScrollBarNode 的 value/range 协议，因此
 * Panel、列表、树控件可以复用同一套 offset 状态机。
 */
class ScrollBehavior final : public UIBehavior {
public:
  ScrollBehavior() = default;

  /** 绑定复合视觉并安装滚动条 value 回调；旧绑定会被下一次绑定替换。 */
  void BindVertical(BaseNode *viewport, BaseNode *content,
                    ScrollBarNode *scrollBar);
  /** 配置和偏移访问器统一维持 [0, maximum] 范围不变量。 */
  const ScrollProperty &GetProperties() const { return Properties; }
  void SetProperties(const ScrollProperty &properties);
  float GetOffsetY() const { return OffsetY; }
  float GetMaximumOffsetY() const { return MaximumOffsetY; }
  void SetOffsetY(float offset);

  /** 滚轮、布局和属性变化最终都汇入同一范围与视觉同步流程。 */
  bool OnMouseWheel(MouseWheelArgs args) override;
  void OnLayoutUpdated() override;
  bool SetProperty(const std::string &name, const std::string &value) override;

protected:
  void OnDetached() override;

private:
  void ApplyProperties();
  void SynchronizeVisuals();
  void UpdateRange();

  ScrollProperty Properties;
  BaseNode *Viewport = nullptr;
  BaseNode *Content = nullptr;
  ScrollBarNode *VerticalScrollBar = nullptr;
  float OffsetY = 0.0f;
  float MaximumOffsetY = 0.0f;
};

} // namespace z8::ui
