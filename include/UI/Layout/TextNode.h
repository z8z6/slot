#pragma once

#include "UI/Layout/BaseNode.h"

#include <DirectXMath.h>
#include <string>
#include <vector>

namespace z8::ui {

/** 文本水平对齐方式；垂直方向由原生布局框和行高共同约束。 */
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
  /** 弹层相交时保存文字仍可绘制的矩形片段；空集合也可表示完全遮挡。 */
  std::vector<DirectX::XMFLOAT4> VisibleTextClips;
  bool HasTextOcclusion = false;

  TextNode();
  explicit TextNode(std::string text);
  const char *TypeName() const override { return "Text"; }
  bool SetProperty(const std::string &name, const std::string &value) override;
};

} // namespace z8::ui
