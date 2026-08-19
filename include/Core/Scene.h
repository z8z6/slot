#pragma once

#include "../Object/GameObject.h"
#include "Util/Owner.h"

namespace z8 {

class BaseCamera;
class BaseLight;

/**
 * @brief 一个可独立装载和销毁的场景，统一拥有相机、多光源和场景对象。
 */
class Scene {
public:
  Owner<BaseCamera> Camera;
  OwnerArray<BaseLight> Lights;
  OwnerArray<GameObject> GOs;

  Scene();
  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;
};

} // namespace z8
