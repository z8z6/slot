//
// Created by zhou_zhengming on 2026/5/20.
//

#pragma once

#include "GameObject.h"

namespace z8 {
/**
 * @brief 平凡的游戏物体
 * @details 默认包含世界矩阵的常量
 */
class SimpleGameObject : public GameObjectImpl<ObjectTransformConst> {
public:
  void Update(Timer*) override;
};
}




