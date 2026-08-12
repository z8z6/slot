#pragma once

#include "UI/Layout/BaseNode.h"

#include <DirectXMath.h>
#include <string>

namespace z8::ui {

/** 文本水平对齐方式；垂直方向由 Yoga 节点框和行高共同约束。 */
enum class TextAlignment { Leading, Center, Trailing };

/**
 * 由文字渲染通道消费的纯布局节点。
 *
 * TextNode 不创建矩形 UIObject，也不携带 Behavior；它只描述排版输入和布局后
 * 的像素框，使 DirectWrite 可以在 DX12 UI 几何之后绘制清晰的系统文字。
 */
class TextNode : public BaseNode {
public:
  std::string Text;
  std::wstring FontFamily = L"Segoe UI";
  float FontSize = 16.0f;
  DirectX::XMFLOAT4 Color = {0.95f, 0.95f, 0.95f, 1.0f};
  TextAlignment Alignment = TextAlignment::Leading;
  bool Wrap = false;

  TextNode();
  explicit TextNode(std::string text);
  const char *TypeName() const override { return "Text"; }
  bool SetProperty(const std::string &name, const std::string &value) override;
};

} // namespace z8::ui
