//
// Created by zhou_zhengming on 2026/5/22.
//

#include "Target/DirectX/DX12GlobalConst.h"
#include "Core/Timer.h"
#include "Target/DirectX/DX12Render.h"
#include "UI/Object/Camera.h"

using namespace z8;
using namespace DirectX;

void DX12GlobalConst::Update(DX12Render* R) {
  XMMATRIX vp = XMLoadFloat4x4(&R->GetCamera()->GetViewProj());
  XMStoreFloat4x4(&ViewProj, XMMatrixTranspose(vp));

  TimeCost = R->GetTimer()->TimeCost;
  TimeTotal = R->GetTimer()->TimeTotal;

  memcpy(R->ConstBuf.GetCPUOffset(Index), this, sizeof(DX12GlobalConst));
}