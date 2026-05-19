//
// Created by zhou_zhengming on 2026/5/17.
//

#include "Target/DirectX/DX12Msaa.h"
#include <cassert>
#include <d3d12.h>
#include "Target/DirectX/DX12Device.h"

using namespace z8;

void z8::DX12Msaa::Init()
{
  // 查询 MSAA 的支持情况
  D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS QL;
  QL.Format = Format;
  QL.SampleCount = SampleCount;
  QL.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
  QL.NumQualityLevels = 0;
  Ok(Ctx->Device->CheckFeatureSupport(
    D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
    &QL,
    sizeof(QL)));

  MsaaQuality = QL.NumQualityLevels;
  assert(MsaaQuality > 0 && "Unexpected MSAA quality level.");
}

unsigned DX12Msaa::GetMsaaQuality() const {
  return EnableMsaa ? (MsaaQuality - 1) : 0;
}

unsigned DX12Msaa::GetSampleCount() const {
  return EnableMsaa ? SampleCount : 1;
}