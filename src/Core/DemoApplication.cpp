//
// Created by zhou_zhengming on 2026/5/19.
//

#include "Core/DemoApplication.h"

#include "Light/ParallelLight.h"
#include "Object/Camera/Camera.h"
#include "Object/Camera/FirstPersonCamera.h"
#include "Object/GameObject/CubeObject.h"
#include "Object/GameObject/SphereObject.h"
#include "Target/Render.h"

#include <stdexcept>

using namespace z8;
using namespace z8::ui;

void DemoApplication::Init() {
  auto camera = std::make_unique<FirstPersonCamera>();
  ActiveScene.SetCamera(std::move(camera));
  auto light = std::make_unique<ParallelLight>();
  light->Transform.Position = { 0, 2, -5};
  light->Direction = { 0.57735f, -0.57735f, 0.57735f };
  ActiveScene.SetLight(std::move(light));

  PrepareScene();
  Render = Render::CreateRender(this);
  Render->Init();
  Window.Open();
}

void DemoApplication::PrepareScene() {
  for (int i = -2; i < 3; i++) {
    if (i == 0) continue;
    auto& c = ActiveScene.CreateGameObject<CubeObject>();
    c.Transform.Position.x = 10.0f * i;
    c.Transform.Scale.x *= 2;
    c.Transform.Scale.y *= 5;
    c.Transform.Scale.z *= 2;
  }

  // 硬边立方体的单个平面不会形成局部法线变化；中心球体用于直接观察逐像素
  // Blinn-Phong 高光，并同时覆盖解析球面法线的渲染路径。
  auto& sphere = ActiveScene.CreateGameObject<SphereObject>();
  sphere.Transform.Scale = {0.5f, 0.5f, 0.5f};

  // UI 声明完全位于 asset/xml；首次加载与后续热重载共享同一事务解析路径。
  // 初始文件缺失无法构成可用 Demo，因此用英文异常明确终止初始化。
  if (!EnableXamlHotReload("asset/xml/Main.xaml"))
    throw std::runtime_error("Unable to load asset/xml/Main.xaml.");
}
