//
// Created by zhou_zhengming on 2026/8/19.
//

#pragma once

#include "BuiltinResource.h"
#include "UIObject.h"

namespace z8 {
struct RectUIObject : UIObject {
  RectUIObject() {
    Renderable.Mesh = ResourceRef<BaseMesh>(builtin::mesh::RectMesh);
  }
};

struct SimpleGameObject : GameObjectImpl<ObjectTransformConst> {
  void Update(Timer*) override;
};

struct CubeObject : SimpleGameObject{
  CubeObject() {
    Renderable.Mesh = ResourceRef<BaseMesh>(builtin::mesh::CubeMesh);
  }
};

struct GridObject : SimpleGameObject {
  GridObject() {
    Renderable.Mesh = ResourceRef<BaseMesh>(builtin::mesh::GridMesh);
  }
};

struct MountainObject : SimpleGameObject {
  MountainObject() {
    Renderable.Mesh = ResourceRef<BaseMesh>(builtin::mesh::MountainMesh);
  }
};

struct RectObject : SimpleGameObject{
  RectObject() {
    Renderable.Mesh = ResourceRef<BaseMesh>(builtin::mesh::RectMesh);
  }
};

struct RotateCube : CubeObject{
  RotateCube();
  EventReply OnMouseMove(MouseMovArgs) override;
};

struct SkullObject : SimpleGameObject{
  SkullObject() {
    Renderable.Mesh = ResourceRef<BaseMesh>(builtin::mesh::SkullMesh);
  }
};

struct SphereObject : SimpleGameObject {
  SphereObject() {
    Renderable.Mesh = ResourceRef<BaseMesh>(builtin::mesh::SphereMesh);
  }
};
}
