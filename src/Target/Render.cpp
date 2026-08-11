//
// Created by zhou_zhengming on 2026/5/12.
//
#include "Target/Render.h"
#include "Target/DirectX/DX12Render.h"
#include "dxgi1_4.h"

using namespace z8;

std::unique_ptr<Render> Render::CreateRender(Application *App,
                                             RenderType type)
{
  return std::make_unique<DX12Render>(App);
}
