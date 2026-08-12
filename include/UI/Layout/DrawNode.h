#pragma once

#include "UI/Layout/BehaviorNode.h"

#include <memory>

namespace z8 {
class UIObject;
}

namespace z8::ui {

/**
 * DrawNode 同时参与布局和绘制的 UI 节点中间层。
 */
class DrawNode : public BehaviorNode {
public:
  std::unique_ptr<UIObject> UO;

  explicit DrawNode(std::unique_ptr<UIObject> O);
  ~DrawNode() override;

  bool SetProperty(const std::string &name, const std::string &value) override;
  bool SetColor(const DirectX::XMFLOAT4 &color) const;
  /** 设置像素宽度边框；0 表示不绘制边框。 */
  bool SetBorder(const DirectX::XMFLOAT4 &color, float width) const;
  void Synchronize() override;
};

} // namespace z8::ui
