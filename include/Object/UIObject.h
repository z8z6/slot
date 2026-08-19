//
// Created by zhou_zhengming on 2026/5/17.
//

#pragma once
#include "GameObject.h"

#include <cstddef>

namespace z8
{
/** UI 每对象常量；颜色放在世界矩阵之后，与 HLSL b0 布局保持一致。 */
struct UIObjectConst {
  // 与所有 b0 Shader 共享相同前缀，后续字段才能在同一根签名下保持稳定偏移。
  ObjectTransformConst Matrices;
  DirectX::XMFLOAT4 Color = {0.35f, 0.35f, 0.38f, 1.0f};
  // left/top/right/bottom 像素裁剪框；由 Layout 逐层求交后写入。
  DirectX::XMFLOAT4 ClipRect = {-100000.0f, -100000.0f,
                                100000.0f, 100000.0f};
  DirectX::XMFLOAT4 BorderColor = {0.0f, 0.0f, 0.0f, 0.0f};
  // left/top/right/bottom 屏幕像素边界让 Shader 能按固定像素宽度绘制边框。
  DirectX::XMFLOAT4 RectBounds{};
  float BorderWidth = 0.0f;
  float CornerRadius = 0.0f;
  float VisualType = 0.0f;
  float ImageKind = 0.0f;
};
static_assert(offsetof(UIObjectConst, Color) == 128,
              "UI color must follow the shared transform matrices.");
static_assert(sizeof(UIObjectConst) == 208,
              "UIObjectConst must match asset/shader/Core/Const.hlsl b0 layout.");

/**
 * @brief 屏幕 UI 层的物体
 * @details 包含默认的 Transform 常量
 * @note UI 层物体的坐标计算逻辑不同
 */
class UIObject : public GameObjectImpl<UIObjectConst>{
public:
  UIObject();
  void SetPosition(float x, float y, float w, float h);
  void SetScale(float x, float y);
  void SetColor(const DirectX::XMFLOAT4& color) { Const.Color = color; }
  const DirectX::XMFLOAT4& GetColor() const { return Const.Color; }
  void SetClipRect(const DirectX::XMFLOAT4& clipRect) {
    Const.ClipRect = clipRect;
  }
  const DirectX::XMFLOAT4& GetClipRect() const { return Const.ClipRect; }
  void SetBorder(const DirectX::XMFLOAT4 &color, float width) {
    Const.BorderColor = color;
    Const.BorderWidth = width;
  }
  const DirectX::XMFLOAT4 &GetBorderColor() const {
    return Const.BorderColor;
  }
  float GetBorderWidth() const { return Const.BorderWidth; }
  float GetCornerRadius() const { return Const.CornerRadius; }
  void SetCornerRadius(float radius) { Const.CornerRadius = radius; }
  void SetImageKind(float kind) {
    Const.VisualType = 1.0f;
    Const.ImageKind = kind;
  }
  void Update(Timer*) override;
};
}

