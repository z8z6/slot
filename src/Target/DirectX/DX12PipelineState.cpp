//
// Created by zhou_zhengming on 2026/5/12.
//

#include "Target/DirectX/DX12PipelineState.h"
#include "Target/DirectX/DX12Device.h"
#include "Target/DirectX/DX12Render.h"
#include "Target/DirectX/DX12Shader.h"
#include "UI/Object/GameObject/GameObject.h"
#include "UI/Shader/Shader.h"
#include "Util/Error.h"
#include "d3d12.h"
#include "d3dx12.h"

using namespace z8;

z8::DX12PipelineState::DX12PipelineState(DX12Render* R) : DX12Common(R)
{
}

void DX12PipelineState::Init(GameObject* O)
{
  InputLayout =
  {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
  };

  D3D12_GRAPHICS_PIPELINE_STATE_DESC PD;
  ZeroMemory(&PD, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
  PD.InputLayout = {InputLayout.data(), static_cast<UINT>(InputLayout.size())};
  PD.pRootSignature = Render->RootSignature.Get();
  // @todo 不同物体的 shader 需要不同的 PSO
  PD.VS = O->VertexShader->Binary->GetByteCode();
  PD.PS = O->PixelShader->Binary->GetByteCode();
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
  Ok(Ctx->Device->CreateGraphicsPipelineState(&PD, IID_PPV_ARGS(&NormalPipe)));

  // 线框模式绘制
  auto WD = PD;
  WD.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
  Ok(Ctx->Device->CreateGraphicsPipelineState(&WD, IID_PPV_ARGS(&WireFramePipe)));
}

ID3D12PipelineState *DX12PipelineState::Get() const {
  return NormalPipe.Get();
}

ID3D12PipelineState * DX12PipelineState::operator->() const
{
  return NormalPipe.Get();
}

void DX12PipelineState::UpdatePSOTy() {
  if(GetAsyncKeyState('1') & 0x8000)
    PSOTy = Default;
  if(GetAsyncKeyState('2') & 0x8000)
    PSOTy = WireFrame;
}

void DX12PipelineState::Set() {
  UpdatePSOTy();
  if(PSOTy == WireFrame)
    Render->Cmd.List->SetPipelineState(WireFramePipe.Get());
  if(PSOTy == Default)
    Render->Cmd.List->SetPipelineState(NormalPipe.Get());
}