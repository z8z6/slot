#include "UI/Layout/VisualNode.h"

#include "Object/UIObject/UIObject.h"
#include "UI/Style/UITheme.h"

#include <stdexcept>
#include <utility>

using namespace z8::ui;

VisualNode::VisualNode(std::unique_ptr<UIObject> visual)
    : Visual(std::move(visual)) {
  if (!Visual)
    throw std::invalid_argument("VisualNode requires a UIObject.");
}

VisualNode::~VisualNode() {
  // C++ 会先执行派生析构体、再销毁 Visual、最后进入 BaseNode 析构；显式提前
  // 释放 Behavior 才能兑现其可在 OnDetached 中安全观察视觉资源的生命周期约束。
  ReleaseBehaviors();
}

bool VisualNode::SetProperty(const std::string &name,
                             const std::string &value) {
  if (name != "Color")
    return BaseNode::SetProperty(name, value);
  DirectX::XMFLOAT4 color;
  if (!ParseUIColor(value, color))
    return false;
  return SetColor(color);
}

bool VisualNode::SetColor(const DirectX::XMFLOAT4 &color) {
  // 构造约束保证 Visual 有效；保留检查可防御未来自定义节点错误转移所有权。
  if (!Visual)
    return false;
  Visual->SetColor(color);
  return true;
}

void VisualNode::SynchronizeVisual(const DirectX::XMFLOAT4 &clip) {
  if (!Visual)
    return;
  Visual->SetPosition(Left, Top, Width, Height);
  Visual->SetScale(Width, Height);
  Visual->SetClipRect(Visible ? clip : DirectX::XMFLOAT4{0, 0, 0, 0});
}
