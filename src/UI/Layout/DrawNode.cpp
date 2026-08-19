#include "UI/Layout/DrawNode.h"

#include "../../../include/Object/UIObject.h"
#include "UI/Property/PropertyParser.h"
#include "UI/Style/Theme.h"
#include "Util/Color.h"

#include <stdexcept>
#include <utility>

using namespace z8::ui;

DrawNode::DrawNode(std::unique_ptr<UIObject> O) : UO(std::move(O)) {
  // 空渲染对象属于可诊断的构造错误，不能用 assert 直接终止宿主进程；
  // 抛出异常也让声明式 UI 能在边界处补充节点来源等上下文。
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
  if (name == "CornerRadius" || name == "Radius") {
    float radius = 0.0f;
    return ParseFiniteFloat(value, radius) && radius >= 0.0f &&
           SetCornerRadius(radius);
  }
  if (name == "Border" || name == "BorderWidth") {
    float width = 0.0f;
    if (!ParseFiniteFloat(value, width) || width < 0.0f)
      return false;
    UO->SetBorder(UO->GetBorderColor(), width);
    return true;
  }
  if (name == "BorderColor") {
    DirectX::XMFLOAT4 color;
    if (!ParseUIColor(value, color))
      return false;
    UO->SetBorder(color, UO->GetBorderWidth());
    return true;
  }
  if (name != "Color")
    return BehaviorNode::SetProperty(name, value);
  DirectX::XMFLOAT4 color;
  if (!ParseUIColor(value, color))
    return false;
  return SetColor(color);
}

bool DrawNode::SetBorder(const DirectX::XMFLOAT4 &color, float width) const {
  assert(UO);
  UO->SetBorder(color, width);
  return true;
}

bool DrawNode::SetColor(const DirectX::XMFLOAT4 &color) const {
  assert (UO);
  UO->SetColor(color);
  return true;
}

bool DrawNode::SetCornerRadius(float radius) const {
  assert(UO);
  if (radius < 0.0f)
    return false;
  UO->SetCornerRadius(radius);
  return true;
}

void DrawNode::Synchronize() {
  assert (UO);
  UO->SetPosition(Left, Top, Width, Height);
  UO->SetScale(Width, Height);
  UO->SetClipRect(EffectiveVisible ? VisibleClip
                                  : DirectX::XMFLOAT4{0, 0, 0, 0});
}
