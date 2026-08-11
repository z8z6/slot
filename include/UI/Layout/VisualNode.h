#pragma once

#include "UI/Layout/BaseNode.h"

#include <memory>

namespace z8 {
class UIObject;
}

namespace z8::ui {

/**
 * 同时参与布局和绘制的 UI 节点中间层。
 *
 * VisualNode 是布局树与渲染层之间唯一的所有权边界：BaseNode 完全不知道
 * UIObject，Visual 则由 VisualNode 独占。公开成员用于框架内部直接遍历，避免
 * 为简单可见状态增加大量 Get 包装；所有权仍由 unique_ptr 明确约束。
 */
class VisualNode : public BaseNode {
public:
  std::unique_ptr<UIObject> Visual;

  /** 每个 VisualNode 构造时必须得到有效视觉，防止产生无法绘制的半成品节点。 */
  explicit VisualNode(std::unique_ptr<UIObject> visual);
  ~VisualNode() override;

  /** 视觉属性在此终止，非视觉 BaseNode 不接受 Color。 */
  bool SetProperty(const std::string &name, const std::string &value) override;
  /** 同一入口供主题、XAML 和即时声明更新每对象颜色常量。 */
  bool SetColor(const DirectX::XMFLOAT4 &color);
  /** 把布局缓存一次性写入位置、缩放和裁剪常量，保持渲染与命中一致。 */
  void SynchronizeVisual(const DirectX::XMFLOAT4 &clip) override;
};

} // namespace z8::ui
