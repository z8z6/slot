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

  D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
  ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
  psoDesc.InputLayout = {InputLayout.data(), static_cast<UINT>(InputLayout.size())};
  psoDesc.pRootSignature = Render->RootSignature.Get();
  psoDesc.VS = Render->GetObjects()->Material->V->GetByteCode();
  psoDesc.PS = Render->GetObjects()->Material->P->GetByteCode();
  psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
  psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
  psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
  psoDesc.SampleMask = UINT_MAX;
  psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  psoDesc.NumRenderTargets = 1;
  psoDesc.RTVFormats[0] = Render->RenderTarget.Format;
  psoDesc.SampleDesc.Count = Render->Msaa.GetSampleCount();
  psoDesc.SampleDesc.Quality = Render->Msaa.GetMsaaQuality();
  psoDesc.DSVFormat = Render->DepthStencil.Format;
  Ok(Ctx->Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&Pipe)));
}

ID3D12PipelineState* DX12PipelineState::operator->() const
{
  return Pipe.Get();
}
