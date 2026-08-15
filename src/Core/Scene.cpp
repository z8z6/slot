#include "Core/Scene.h"

#include "Light/ParallelLight.h"
#include "Object/Camera/BaseCamera.h"

using namespace z8;

Scene::Scene() {
  Camera.set<BaseCamera>();
  Lights.add<ParallelLight>();
}
