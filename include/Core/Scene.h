#pragma once

#include "Object/GameObject/GameObject.h"
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
  /** 光源按稳定声明顺序上传；超过后端上限的尾部光源不会参与当前帧。 */
  OwnerArray<BaseLight> Lights;
  OwnerArray<GameObject> GOs;

  Scene();
  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;
};

} // namespace z8
