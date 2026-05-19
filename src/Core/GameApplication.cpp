//
// Created by zhou_zhengming on 2026/5/19.
//

#include "Core/GameApplication.h"

#include "Target/Render.h"
#include "UI/Object/Camera.h"
#include "UI/Object/CubeObject.h"
#include "UI/Object/FirstPersonCamera.h"

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
  for (int i = -2; i < 3; i++) {
    auto* c = new CubeObject();
    c->Transform.Position.x = 10.0f * i;
    Objects.push_back(c);
  }
}