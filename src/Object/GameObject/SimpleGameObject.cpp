//
// Created by zhou_zhengming on 2026/5/20.
//

#include "Object/GameObject/SimpleGameObject.h"
#include "Object/Camera/Camera.h"

using namespace z8;
void SimpleGameObject::Update(Timer* T) {
  // 世界矩阵与法线逆转置矩阵必须来自同一次 Transform 更新，避免一帧内空间不一致。
  Transform.UpdateWorld();
  Const.Update(Transform.World);
}
