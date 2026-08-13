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
#include "UI/Layout/SceneNode.h"
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
  if (ui.BeginPanel("toolbar", "Level Editor"))
    ui.EndPanel();
  ui.Terminal("terminal", "Output Log");
  if (ui.BeginPanel("outliner", "World Outliner")) {
    UIStyle itemStyle;
    itemStyle.Height = demoStyle.RowHeight;
    itemStyle.FlexGrow = 0.0f;
    itemStyle.FlexShrink = 0.0f;
    itemStyle.Margin = demoStyle.RowMargin;
    for (int i = 0; i < 8; ++i) {
      // 第一个条目模拟 UE 编辑器的非焦点选择，其余使用主题定义的交替行色。
      itemStyle.Color = i == 0 ? demoStyle.SelectedRowColor
                        : i % 2 == 0 ? demoStyle.RowColor
                                     : demoStyle.AlternateRowColor;
      ui.Rect("outliner-item-" + std::to_string(i), itemStyle);
    }
    ui.EndPanel();
  }
  if (ui.BeginPanel("details", "Details"))
    ui.EndPanel();
  if (auto *scene = ui.Scene("scene-viewport"))
    scene->SetProperty("Title", "Perspective");
  ui.EndFrame();

  // UE 编辑器式工作区从四边切割工具面板，中央剩余区域由 SceneNode 占满。
  const auto configureDock = [this](const char *key, const char *placement,
                                    float extent) {
    if (auto *node = Layout.Find(key)) {
      node->SetProperty("Dock", placement);
      node->SetProperty("DockExtent", std::to_string(extent));
    }
  };
  configureDock("toolbar", "Top", demoStyle.ToolbarHeight);
  // 工具栏允许从 48px 向上或向下调整；不能继承普通内容 Panel 的 160px
  // 最小高度，否则初始 DockExtent 与 ResizeBehavior 约束互相矛盾。
  if (auto *toolbar = Layout.Find("toolbar"))
    toolbar->SetProperty("MinHeight",
                         std::to_string(Theme::Default().Panel.TitleHeight));
  configureDock("terminal", "Bottom", demoStyle.BottomPanelHeight);
  configureDock("outliner", "Left", demoStyle.LeftPanelWidth);
  configureDock("details", "Right", demoStyle.RightPanelWidth);
}
