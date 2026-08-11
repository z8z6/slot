#include "Core/Scene.h"

#include "Light/Light.h"
#include "Object/Camera/Camera.h"

using namespace z8;

Scene::Scene() = default;
Scene::~Scene() = default;

void Scene::SetCamera(std::unique_ptr<Camera> camera) {
  ActiveCamera = std::move(camera);
}

void Scene::SetLight(std::unique_ptr<Light> light) {
  MainLight = std::move(light);
}
