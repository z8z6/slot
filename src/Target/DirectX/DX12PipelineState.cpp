//
// Created by zhou_zhengming on 2026/5/12.
//

#include "Target/DirectX/DX12PipelineState.h"
#include "d3d12.h"
#include "d3dx12.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"
#include "UI/Object/GameObject.h"
#include "UI/Material/Material.h"
#include "Target/DirectX/DX12Shader.h"

using namespace z8;

z8::DX12PipelineState::DX12PipelineState(DX12Render* R) : DX12Common(R)
{
}

void DX12PipelineState::Init()
{
  InputLayout =
  {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
  };

  D3D12_GRAPHICS_PIPELINE_STATE_DESC PD;
  ZeroMemory(&PD, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
  PD.InputLayout = {InputLayout.data(), static_cast<UINT>(InputLayout.size())};
  PD.pRootSignature = Render->RootSignature.Get();
  // @todo
  PD.VS = Render->RenderObjects[0].Object->Material->V->GetByteCode();
  PD.PS = Render->RenderObjects[0].Object->Material->P->GetByteCode();
  PD.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  PD.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  PD.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
  PD.SampleMask = UINT_MAX;
  PD.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  PD.NumRenderTargets = 1;
  PD.RTVFormats[0] = Render->RenderTarget.Format;
  PD.SampleDesc.Count = Render->Msaa.GetSampleCount();
  PD.SampleDesc.Quality = Render->Msaa.GetMsaaQuality();
  PD.DSVFormat = Render->DepthStencil.Format;
  Ok(Ctx->Device->CreateGraphicsPipelineState(&PD, IID_PPV_ARGS(&Pipe)));
}

ID3D12PipelineState *DX12PipelineState::Get() const {
  return Pipe.Get();
}

ID3D12PipelineState * DX12PipelineState::operator->() const
{
  return Pipe.Get();
}
