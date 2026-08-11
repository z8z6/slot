//
// Created by zhou_zhengming on 2026/5/17.
//

#pragma once
#include "Object/GameObject/GameObject.h"

namespace z8
{
/** UI 每对象常量；颜色放在世界矩阵之后，与 HLSL b0 布局保持一致。 */
struct UIObjectConst {
  DirectX::XMFLOAT4X4 World;
  DirectX::XMFLOAT4 Color = {0.35f, 0.35f, 0.38f, 1.0f};
  // left/top/right/bottom 像素裁剪框；由 Layout 逐层求交后写入。
  DirectX::XMFLOAT4 ClipRect = {-100000.0f, -100000.0f,
                                100000.0f, 100000.0f};
};

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
  void Update(Timer*) override;
};
}

