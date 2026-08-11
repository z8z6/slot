//
// Created by zhou_zhengming on 2026/5/17.
//

#include "Object/GameObject/GameObject.h"
#include "Resource/BuiltinResource.h"

z8::GameObject::GameObject()
    : Collider(nullptr) {
  // 普通场景对象共享默认材质和 Program；派生对象只需要选择几何资源。
  Renderable.Material = ResourceReference<Material>(builtin::MetalMaterial);
}

z8::GameObject::~GameObject() = default;
