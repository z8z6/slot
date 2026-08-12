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
#include "UI/Style/Theme.h"

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
    auto& c = ActiveScene.CreateGameObject<CubeObject>();
    c.Transform.Position.x = 10.0f * i;
    c.Transform.Scale.x *= 2;
    c.Transform.Scale.y *= 5;
    c.Transform.Scale.z *= 2;
  }

  // ImGui 风格声明会生成并保留控件树；稳定 key 使后续重复声明能够复用控件。
  ImmediateUI ui(Layout);
  ui.BeginFrame();
  const auto &demoStyle = Theme::Default().Demo;
  UIStyle panelStyle;
  panelStyle.Width = demoStyle.PanelWidth;
  panelStyle.Height = demoStyle.PanelHeight;
  if (ui.BeginPanel("scene-panel", "Scrollable Scene Objects", panelStyle)) {
    UIStyle itemStyle;
    itemStyle.Height = demoStyle.RowHeight;
    itemStyle.FlexGrow = 0.0f;
    itemStyle.FlexShrink = 0.0f;
    itemStyle.Margin = demoStyle.RowMargin;
    for (int i = 0; i < 14; ++i) {
      // 第一个条目模拟 UE 编辑器的非焦点选择，其余使用主题定义的交替行色。
      itemStyle.Color = i == 0 ? demoStyle.SelectedRowColor
                        : i % 2 == 0 ? demoStyle.RowColor
                                     : demoStyle.AlternateRowColor;
      ui.Rect("scene-item-" + std::to_string(i), itemStyle);
    }
    ui.EndPanel();
  }
  ui.EndFrame();
}
