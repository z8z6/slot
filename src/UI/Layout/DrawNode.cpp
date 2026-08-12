#include "UI/Layout/DrawNode.h"

#include "Object/UIObject/UIObject.h"
#include "UI/Style/UITheme.h"

#include <stdexcept>
#include <utility>

using namespace z8::ui;

DrawNode::DrawNode(std::unique_ptr<UIObject> O) : UO(std::move(O)) {
  if (!UO)
    throw std::invalid_argument("DrawNode requires a render object.");
  HitTestVisible = true;
}

DrawNode::~DrawNode() {
  // C++ 会先执行派生析构体、再销毁 Visual、最后进入 BaseNode 析构；显式提前
  // 释放 Behavior 才能兑现其可在 OnDetached 中安全观察视觉资源的生命周期约束。
  ReleaseBehaviors();
}

bool DrawNode::SetProperty(const std::string &name,
                             const std::string &value) {
  if (name != "Color")
    return BehaviorNode::SetProperty(name, value);
  DirectX::XMFLOAT4 color;
  if (!ParseUIColor(value, color))
    return false;
  return SetColor(color);
}

bool DrawNode::SetColor(const DirectX::XMFLOAT4 &color) const {
  assert (UO);
  UO->SetColor(color);
  return true;
}

void DrawNode::Synchronize() {
  assert (UO);
  UO->SetPosition(Left, Top, Width, Height);
  UO->SetScale(Width, Height);
  UO->SetClipRect(Visible ? VisibleClip : DirectX::XMFLOAT4{0, 0, 0, 0});
}
