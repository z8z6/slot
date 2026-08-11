//
// Created by zhou_zhengming on 2026/5/19.
//

#include "Core/DemoApplication.h"

#include "Light/ParallelLight.h"
#include "Object/Camera/Camera.h"
#include "Object/Camera/FirstPersonCamera.h"
#include "Object/GameObject/CubeObject.h"
#include "Object/UIObject/RectUIObject.h"
#include "Target/Render.h"
#include "UI/Declarative/ImmediateUI.h"
#include "UI/Layout/RectNode.h"

using namespace z8;
using namespace z8::ui;
using namespace DirectX;

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
    auto& c = ActiveScene.CreateGameObject<CubeObject>();
    c.Transform.Position.x = 10.0f * i;
    c.Transform.Scale.x *= 2;
    c.Transform.Scale.y *= 5;
    c.Transform.Scale.z *= 2;
  }

  // ImGui 风格声明会生成并保留控件树；稳定 key 使后续重复声明能够复用控件。
  ImmediateUI ui(Layout);
  ui.BeginFrame();
  UIStyle panelStyle;
  panelStyle.Width = 360.0f;
  panelStyle.Height = 240.0f;
  panelStyle.Padding = 8.0f;
  if (ui.BeginPanel("scene-panel", "Scene", panelStyle)) {
    UIStyle itemStyle;
    itemStyle.FlexGrow = 1.0f;
    itemStyle.Margin = 4.0f;
    ui.Rect("scene-item-1", itemStyle);
    ui.Rect("scene-item-2", itemStyle);
    ui.EndPanel();
  }
  ui.EndFrame();
}
