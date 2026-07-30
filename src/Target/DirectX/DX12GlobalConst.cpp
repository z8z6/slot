//
// Created by zhou_zhengming on 2026/5/22.
//

#include "Target/DirectX/DX12GlobalConst.h"
#include "../../../include/UI/Object/Camera/Camera.h"
#include "Core/Timer.h"
#include "Core/Window.h"
#include "Target/DirectX/DX12Render.h"
#include "UI/Light/Light.h"

using namespace z8;
using namespace DirectX;

void DX12GlobalConst::Update(DX12Render* R) {
  // 投影矩阵
  XMMATRIX vp = XMLoadFloat4x4(&R->GetCamera()->GetViewProj());
  XMStoreFloat4x4(&ViewProj, XMMatrixTranspose(vp));

  AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };
  // 相机
  Camera = R->GetCamera()->Transform.Position;

  Light = {};
  Light.Position = R->GetLight()->Transform.Position;
  Light.Strength = R->GetLight()->Color;
  Light.Direction = R->GetLight()->Direction;

  ScreenSize.x = static_cast<float>(R->GetWindow()->Width);
  ScreenSize.y = static_cast<float>(R->GetWindow()->Height);

  TimeCost = R->GetTimer()->TimeCost;
  TimeTotal = R->GetTimer()->TimeTotal;

  // 写入常量缓冲区
  WriteToBuffer(R);
}

unsigned DX12GlobalConst::AlignedSize() {
  return DX12ConstBuffer::AlignedSize(sizeof(DX12GlobalConst));
}

void DX12GlobalConst::WriteToBuffer(DX12Render* R) const {
  auto index = R->GOBatch.Buffer.GetGlobalConstIndex();
  auto offset = R->GOBatch.Buffer.GetCPUOffset(index);
  memcpy(offset, this, sizeof(DX12GlobalConst));

  index = R->UOBatch.Buffer.GetGlobalConstIndex();
  offset = R->UOBatch.Buffer.GetCPUOffset(index);
  memcpy(offset, this, sizeof(DX12GlobalConst));
}