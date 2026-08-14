#pragma once

#include "Object/GameObject/GameObject.h"
#include "Util/Owner.h"


namespace z8 {

class BaseCamera;
class BaseLight;

/**
 * @brief 一个可独立装载和销毁的场景，统一拥有相机、灯光和场景对象。
 */
class Scene {
public:
  Owner<BaseCamera> Camera;
  Owner<BaseLight> Light;
  OwnerArray<GameObject> GOs;

  Scene();
  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;
};

} // namespace z8
