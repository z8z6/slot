//
// Created by zhou_zhengming on 2026/5/17.
//

#pragma once

#include "Object.h"

namespace z8
{
class Mesh;
class Material;
class Collider;
class GameObject : public Object{
public:
  Mesh* Mesh;
  Material* Material;
  Collider* Collider;

  GameObject();

  virtual void* ConstBuf() = 0;
  virtual unsigned ConstBufSize() = 0;
};
}






