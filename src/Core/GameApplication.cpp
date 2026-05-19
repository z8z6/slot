//
// Created by zhou_zhengming on 2026/5/19.
//

#include "Core/GameApplication.h"

#include "Target/Render.h"
#include "UI/Object/Camera.h"
#include "UI/Object/CubeObject.h"
#include "UI/Object/FirstPersonCamera.h"
#include "UI/Object/GridObject.h"

using namespace z8;

void GameApplication::Init() {
  Camera = new FirstPersonCamera();
  PrepareScene();
  Render = Render::CreateRender(this);
  Render->Init();
  Window.Open();
}

void z8::GameApplication::PrepareScene() {
  Camera->Transform.Position.z = -40.0f;
  Camera->Transform.Position.y = 5.0f;
  for (int i = -2; i < 3; i++) {
    auto* c = new CubeObject();
    c->Transform.Position.x = 10.0f * i;
    c->Transform.Scale.x *= 2;
    c->Transform.Scale.y *= 5;
    c->Transform.Scale.z *= 2;
    Objects.push_back(c);
  }
  // auto* g = new GridObject();
  // Objects.push_back(g);
}