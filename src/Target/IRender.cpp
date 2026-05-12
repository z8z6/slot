//
// Created by zhou_zhengming on 2026/5/12.
//
#include "Target/IRender.h"
#include "Target/DirectX/DX12Render.h"

using namespace z8;

IRender* IRender::CreateRender(Application* App, RenderType type)
{
  return new DX12Render(App);
}
