//
// Created by zhou_zhengming on 2026/5/7.
//

#include "Core/Application.h"
#include "DirectX/DX12Render.h"

z8::Application::Application() {
  Render = new DX12Render(&Wnd);
  Render->Init();
  Wnd.Open();
}