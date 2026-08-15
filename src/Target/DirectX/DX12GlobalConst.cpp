//
// Created by zhou_zhengming on 2026/5/22.
//

#include "Target/DirectX/DX12GlobalConst.h"
#include "../../../include/Object/Camera/BaseCamera.h"
#include "Core/Timer.h"
#include "Core/Window.h"
#include "Light/BaseLight.h"
#include "Target/DirectX/DX12Render.h"

#include <algorithm>

using namespace z8;
using namespace DirectX;

void DX12GlobalConst::Update(DX12Render* R) {
  // 投影矩阵
  XMMATRIX vp = XMLoadFloat4x4(&R->GetCamera()->GetViewProj());
  XMStoreFloat4x4(&ViewProj, XMMatrixTranspose(vp));

  AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
  // 相机
  Camera = R->GetCamera()->Transform.Position;

  Lights = {};
  const auto& sceneLights = R->GetLights();
  LightCount = static_cast<uint32_t>((std::min)(
      sceneLights.size(), static_cast<size_t>(MaxLights)));
  for (uint32_t index = 0; index < LightCount; ++index) {
    const auto* light = sceneLights[index];
    if (!light) continue;
    Lights[index].Position = light->Transform.Position;
    Lights[index].Strength = light->Color;
    Lights[index].Direction = light->Direction;
  }

  ScreenSize.x = static_cast<float>(R->GetWindow()->Width);
  ScreenSize.y = static_cast<float>(R->GetWindow()->Height);
  UIOrigin = {0.0f, 0.0f};

  TimeCost = R->GetTimer()->TimeCost;
  TimeTotal = R->GetTimer()->TimeTotal;
  UIScale = R->GetWindow()->DpiScale;

  // 写入常量缓冲区
  WriteToBuffer(R);
}

unsigned DX12GlobalConst::AlignedSize() {
  return DX12ConstBuffer::AlignedSize(sizeof(DX12GlobalConst));
}

void DX12GlobalConst::WriteToBatch(DX12RenderBatch &batch) const {
  const auto index = batch.Buffer.GetGlobalConstIndex();
  memcpy(batch.Buffer.GetCPUOffset(index), this, sizeof(DX12GlobalConst));
}

void DX12GlobalConst::WriteToBuffer(DX12Render* R) const {
  WriteToBatch(R->GOBatch);
  WriteToBatch(R->UOBatch);
}
