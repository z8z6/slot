//
// Created by zhou_zhengming on 2026/5/19.
//

#include "Core/GameApplication.h"

#include "Target/Render.h"
#include "UI/Light/ParallelLight.h"
#include "UI/Object/Camera/Camera.h"
#include "UI/Object/Camera/FirstPersonCamera.h"
#include "UI/Object/GameObject/CubeObject.h"
#include "UI/Object/UIObject/RectUIObject.h"

using namespace z8;
using namespace DirectX;

void GameApplication::Init() {
  Camera = new FirstPersonCamera();
  Light = new ParallelLight();
  Light->Transform.Position = { 0, 2, -5};
  Light->Direction = { 0.57735f, -0.57735f, 0.57735f };

  PrepareScene();
  Render = Render::CreateRender(this);
  Render->Init();
  Window.Open();
}

void z8::GameApplication::PrepareScene() {
  for (int i = -2; i < 3; i++) {
    auto* c = new CubeObject();
    c->Transform.Position.x = 10.0f * i;
    c->Transform.Scale.x *= 2;
    c->Transform.Scale.y *= 5;
    c->Transform.Scale.z *= 2;
    GOs.push_back(c);
  }

  UOs.push_back(new RectUIObject());
}