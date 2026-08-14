//
// Created by zhou_zhengming on 2026/5/17.
//

#include "Object/UIObject/UIObject.h"

#include "Object/Camera/Camera.h"
#include "Resource/BuiltinResource.h"

#include <iostream>
#include <ostream>

using namespace DirectX;
using namespace z8;

z8::UIObject::UIObject() {
  // UI 材质间接选择关闭深度并启用混合的 Program，对象不直接绑定 Shader。
  Renderable.Material = ResourceReference<Material>(builtin::UIMaterial);
}

void UIObject::SetPosition(float x, float y, float w, float h) {
  // 默认网格原点在中心，且变长为 0.5
  // 将左上角坐标转为中心坐标
  Transform.Position.x = x + w / 2;
  Transform.Position.y = y + h / 2;
  Const.RectBounds = {x, y, x + w, y + h};
}

void UIObject::SetScale(float x, float y) {
  // 默认原网格总长度为 1
  Transform.Scale.x = x;
  Transform.Scale.y = y;
}

void z8::UIObject::Update(Timer* T) {
  Transform.UpdateWorld();
  Const.Matrices.Update(Transform.World);
}
