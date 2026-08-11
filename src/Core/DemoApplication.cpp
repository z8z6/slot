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
  panelStyle.Height = 320.0f;
  if (ui.BeginPanel("scene-panel", "Scrollable Scene Objects", panelStyle)) {
    UIStyle itemStyle;
    itemStyle.Height = 48.0f;
    itemStyle.FlexGrow = 0.0f;
    itemStyle.FlexShrink = 0.0f;
    itemStyle.Margin = 4.0f;
    for (int i = 0; i < 14; ++i) {
      // 交替色块让滚动位移和视口裁剪在默认 Demo 中无需文字即可辨认。
      itemStyle.Color = i % 2 == 0
          ? XMFLOAT4{0.20f, 0.34f, 0.52f, 1.0f}
          : XMFLOAT4{0.16f, 0.24f, 0.36f, 1.0f};
      ui.Rect("scene-item-" + std::to_string(i), itemStyle);
    }
    ui.EndPanel();
  }
  ui.EndFrame();
}
