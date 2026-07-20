//
// Created by zhou_zhengming on 2026/5/17.
//

#pragma once
#include "GameObject.h"

namespace z8
{
/**
 * @brief 屏幕 UI 层的物体
 * @details 包含默认的 Transform 常量
 * @note UI 层物体的坐标计算逻辑不同
 */
class UIObject : public GameObjectImpl<DirectX::XMFLOAT4X4>{
public:
  void Update(Timer*) override;
};
}

